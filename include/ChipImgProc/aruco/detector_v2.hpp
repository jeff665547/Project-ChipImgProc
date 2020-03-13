/**
 *  @file    ChipImgProc/aruco/detector.hpp
 *  @author  Chia-Hua Chang, Alex Lee
 *  @brief   @copybrief chipimgproc::aruco::Detector
 *  @details The ArUco marker detection algorithm
 * 
 */
#pragma once
#include <cstdint>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <ChipImgProc/optional.hpp>
#include <ChipImgProc/utils.h>
#include <cinttypes>
#include "utils.hpp"
#include "dictionary.hpp"
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/logger.hpp>
#include "mk_img_dict.hpp"
#include <stdexcept>
#include <cassert>

namespace chipimgproc::aruco {

/**
 *  @brief The ArUco marker detector class.
 *  @details Here shows an example
 *  @snippet ChipImgProc/aruco_test.cpp usage
 */
class Detector2 {
  public:
    Detector2()
      : dictionary_(nullptr)
      , templ_(), stempl_()
      , mask_ (), smask_()
      , anchors_()
      , pyramid_level_(0)
      , nms_count_()
      , nms_radius_()
      , ext_width_()
      , threshold_(3)
      , active_ids_()
      , criteria_(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 300, 1e-3)
      , marker_img_dict_()
    {}
    
    void reset(
        const Dictionary& dictionary
      , const cv::Mat_<uint8_t> border_template
      , const cv::Mat_<uint8_t> border_mask
      , const double aruco_width
      , const std::int32_t pyramid_level
      , const std::int32_t nms_count
      , const double nms_radius
      , const double ext_width
      , const std::vector<std::int32_t>& active_ids = std::vector<std::int32_t>()
      , std::ostream& logger = nucleona::stream::null_out
    ) {
        assert(border_template.depth() == CV_8U);
        assert(border_mask.depth() == CV_8U);
        assert(border_template.size() == border_mask.size());
        assert(aruco_width >= dictionary.coding_bits() * ext_width);

        this->dictionary_ = &dictionary;
        this->templ_ = border_template;
        this->mask_  = border_mask;
        this->pyramid_level_ = pyramid_level;
        this->nms_count_  = nms_count;
        this->nms_radius_ = nms_radius / (1 << pyramid_level);
        this->active_ids_ = active_ids;
        
        auto dsize = border_template.size();
        for (auto i = 0; i < pyramid_level; ++i) {
            dsize.height = (dsize.height + 1) / 2;
            dsize.width  = (dsize.width  + 1) / 2;
        }
        if (pyramid_level > 0) {
            cv::resize(this->templ_, this->stempl_, dsize, 0, 0, cv::INTER_AREA);
            cv::resize(this->mask_ , this->smask_ , dsize, 0, 0, cv::INTER_NEAREST);
        } else {
            this->stempl_ = this->templ_;
            this->smask_  = this->mask_ ;
        }

        auto a = aruco_width / dictionary.coding_bits();
        auto b = 0.5 * (dictionary.coding_bits() - 1);
        auto s = cv::Vec2f(border_template.cols, border_template.rows);
        for (auto i = 0; i != dictionary.coding_bits(); ++i) {
            auto y = a * (i - b) + (s[1] - 1) * 0.5;
            for (auto j = 0; j != dictionary.coding_bits(); ++j) {
                auto x = a * (j - b) + (s[0] - 1) * 0.5;
                this->anchors_.emplace_back(x, y);
            }
        }
        this->anchors_.emplace_back((s[0] - 1) * 0.5, (s[1] - 1) * 0.5);
        this->ext_width_ = std::ceil(ext_width);

        marker_img_dict_.reset(
            dictionary
          , active_ids
          , a
          , dictionary.coding_bits()
          , pyramid_level
        );
    }

    auto aruco_img(std::int32_t mk_index) const {
        return marker_img_dict_.mk_idx_at(mk_index, this->stempl_);
    }

    /**
     *  @brief   Detect markers in an image
     *  @return  A list of marker ids and corresponding locations in pixel scales
     */
    auto detect_markers(
        cv::Mat input,
        std::ostream& logger = nucleona::stream::null_out
    ) const {

        // convert input to 8U image
        cv::Mat_<uint8_t> image;
        if (input.depth() == CV_8U)
            input.copyTo(image);
        else if (input.depth() == CV_16U)
            input.convertTo(image, CV_8U, 255.0 / 16383.0);
        else // invalid input format
            throw std::invalid_argument("Invalid input format");
        
        // roughly fix defects
        image = cv::max(image, cv::mean(image)[0]);

        // apply pyramid downsampling
        cv::Mat_<uint8_t> simage = image.clone();
        for (auto i = 0; i < this->pyramid_level_; ++i)
            cv::pyrDown(simage, simage);
        
        // declare constants
        std::int32_t s = 1 << pyramid_level_;
        cv::Rect roi1(1, 1, simage.cols - 2 * 1, simage.rows - 2 * 1);
        cv::Rect roi2(0, 0, templ_.cols + 2 * s, templ_.rows + 2 * s);
        cv::Mat_<float> match1(roi1.size() - stempl_.size() + cv::Size(1,1));
        cv::Mat_<float> match2(2 * s + 1, 2 * s + 1);

        // search all possible marker locations
        std::cerr << "fine all possible marker locations\n";
        auto best_score = 0.0;
        std::vector<cv::Rect> selections;
        std::vector<cv::Vec2f> new_anchors;
        cv::Mat_<std::uint8_t> new_templ, tp, bw;
        cv::Point_<float> new_center;
        cv::matchTemplate(simage(roi1), stempl_, match1, cv::TM_CCORR_NORMED, smask_);
        for (auto i = 0; i != nms_count_; ++i) {

            // pixel-level roughly search
            cv::Point loc;
            cv::minMaxLoc(match1, nullptr, nullptr, nullptr, &loc);
            roi2.x = loc.x * s;
            roi2.y = loc.y * s;
            cv::circle(match1, loc, nms_radius_, 0, -1);
            
            // pixel-level finely search
            double score;
            cv::matchTemplate(image(roi2), templ_, match2, cv::TM_CCORR_NORMED, mask_);
            cv::minMaxLoc(match2, nullptr, &score, nullptr, &loc);
            cv::Rect roi(roi2.tl() + loc, templ_.size());
            selections.emplace_back(roi);

            // filter
            if (best_score >= score)
                continue;

            // subpixel-level search
            cv::Mat_<uint8_t> view(image, roi);
            cv::Matx23f wmatx = cv::Matx23f::eye();
            std::vector<cv::Vec2f> anchors;
            auto model = cv::MOTION_EUCLIDEAN;
            try {
                cv::findTransformECC(view, templ_, wmatx, model, criteria_, mask_);
                cv::invertAffineTransform(wmatx, wmatx);
                cv::transform(anchors_, anchors, wmatx);
            }
            catch (const cv::Exception& e) {
                continue;
            }

            // aruco decoding
            auto index = -1;
            auto query = this->to_binary_(view, anchors);
            if (dictionary_->identify(query, index, active_ids_)) {
                best_score = score;
                new_templ = view;
                new_anchors = std::move(anchors);
            }
        }

        if (new_templ.empty())
            throw std::runtime_error("new template not found");

        // recognize aruco marks with new template
        std::cerr << "recognize aruco marks with new template\n";
        using K = std::tuple<bool, double>;
        using V = std::tuple<std::int32_t, double, cv::Point2f>;
        std::map<K, V, std::greater<K>> candidates;
        std::vector<V> results;
        for (auto&& roi: selections) {

            // subpixel-level search
            cv::Mat_<uint8_t> view(image, roi);
            cv::Matx23f wmatx = cv::Matx23f::eye();
            auto model = cv::MOTION_TRANSLATION;
            auto score = 0.0;
            try {
                score = cv::findTransformECC(view, new_templ, wmatx, model, criteria_, mask_);
            }
            catch (const cv::Exception& e) {
                continue;
            }

            // aruco decoding
            auto index = -1;
            auto query = this->to_binary_(view, new_anchors);
            auto found = dictionary_->identify(query, index, active_ids_);

            // sort result
            auto k = std::make_tuple(found, score);
            auto v = std::make_tuple(index, score, cv::Point2f(
                roi.x + new_anchors.back()[0] - wmatx(0, 2)
              , roi.y + new_anchors.back()[1] - wmatx(1, 2)
            ));
            candidates[k] = v;
        }

        // export results
        for (auto&& [k, v]: candidates) {
            results.push_back(std::move(v));
        }
        return results;
    }

  private:
    uint64_t to_binary_(
        cv::Mat_<uint8_t> patch
      , const std::vector<cv::Vec2f>& anchors
    ) const {
        auto coding_bits = this->dictionary_->coding_bits();
        auto num_anchors = coding_bits * coding_bits;
        cv::Rect roi(0, 0, ext_width_, ext_width_);
        cv::Mat_<uint8_t> tmp(roi.size() * coding_bits);
        for (auto i = 0; i != num_anchors; ++i) {
            roi.x = roi.width  * (i % coding_bits);
            roi.y = roi.height * (i / coding_bits);
            cv::getRectSubPix(patch, roi.size(), anchors[i], tmp(roi));
        }
        cv::Mat_<uint8_t> bw;
        cv::threshold(tmp, bw, 0, 1, cv::THRESH_BINARY | cv::THRESH_OTSU);
        uint64_t binary = 0ull;
        for (auto i = 0; i != num_anchors; ++i) {
            roi.x = roi.width  * (i % coding_bits);
            roi.y = roi.height * (i / coding_bits);
            uint64_t bit = cv::norm(bw(roi), cv::NORM_L1) > threshold_;
            binary |= bit << i;
        }
        return binary;
    }

  private:
    const Dictionary* dictionary_;
    cv::Mat_<std::uint8_t> templ_, stempl_;
    cv::Mat_<std::uint8_t> mask_ , smask_ ;
    std::vector<cv::Vec2f> anchors_;
    std::int32_t pyramid_level_;
    std::int32_t nms_count_;
    std::int32_t nms_radius_;
    std::int32_t ext_width_;
    std::int32_t threshold_;
    std::vector<std::int32_t> active_ids_;
    cv::TermCriteria criteria_;
    MkImgDict marker_img_dict_; // marker index => image
};

} // namespace aruco
