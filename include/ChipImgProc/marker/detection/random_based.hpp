#pragma once

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <ChipImgProc/utils.h>
#include <Nucleona/language.hpp>
#include <ChipImgProc/logger.hpp>

#include <iostream>

namespace chipimgproc::marker::detection {

struct RndMkId {
    int index;
    int distance;
};
template<class Derived>
struct RandomBased {
private:
    Derived* derived() {
        return static_cast<Derived*>(this);
    }
    const Derived* derived() const {
        return static_cast<const Derived*>(this);
    }
protected:
    RandomBased(
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const std::int32_t&             pyramid_level,
        const double&                   theor_max_val,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius
    )
    : templ_            (templ)
    , stempl_           ()
    , mask_             (mask)
    , smask_            ()
    , pyramid_level_    (pyramid_level)
    , theor_max_val_    (theor_max_val)
    , nms_count_        (nms_count)
    , nms_radius_       (nms_radius)
    , criteria_         (
        cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 300, 1e-5
    )
    , anchors_          ()
    , s_                (1 << pyramid_level)
    , method_           (cv::TM_CCORR_NORMED)
    {
        assert(templ.depth() == CV_8U);
        assert(mask.depth() == CV_8U);
        assert(templ.size() == mask.size());
        auto dsize_w = templ.cols;
        auto dsize_h = templ.rows;

        for(auto i = pyramid_level_; i > 0; i --) {
            dsize_w = (dsize_w + 1) / 2;
            dsize_h = (dsize_h + 1) / 2;
        }
        if(pyramid_level > 0) {
            cv::resize(templ_, stempl_, cv::Size(dsize_w, dsize_h), 0, 0, cv::INTER_AREA);
            cv::resize(mask_,  smask_,  cv::Size(dsize_w, dsize_h), 0, 0, cv::INTER_NEAREST);
        } else {
            stempl_ = templ ;
            smask_  = mask  ;
        }
    }
    RndMkId identify(
        const cv::Mat& view, 
        const std::vector<cv::Vec2f>& anchors
    ) const {
        return {0, 0};
    }
public:
    const auto& templ() {
        return templ_;
    }
    const auto& mask() {
        return mask_;
    }
    const auto& stempl() {
        return stempl_;
    }
    void set_term_criteria(cv::TermCriteria tc) {
        criteria_ = tc;
    }
    template<class... Args>
    std::vector<
        std::tuple<int, double, cv::Point2d>
    > operator()(cv::Mat input, Args&&... identify_args) const {
        // convert input to 8U image
        cv::Mat_<uint8_t> image;
        if (input.depth() == CV_8U)
            input.copyTo(image);
        else if (input.depth() == CV_16U)
            input.convertTo(image, CV_8U, 255.0 / theor_max_val_);
        else // invalid input format
            throw std::invalid_argument("Invalid input format");
        
        // roughly fix defects
        image = cv::max(image, cv::mean(image)[0]);

        // apply pyramid downsampling
        cv::Mat_<uint8_t> simage = image.clone();
        for (auto i = 0; i < this->pyramid_level_; ++i)
            cv::pyrDown(simage, simage);
        {
            auto tmp = simage(cv::Rect(1, 1, simage.cols - 2, simage.rows - 2));
            simage = tmp;
        }

        // search all possible marker locations
        auto best_score = 0.0;
        cv::Mat_<std::uint8_t> new_templ;
        std::vector<cv::Point> locations;
        std::vector<cv::Vec2f> new_anchors;
        auto match1 = chipimgproc::match_template(simage, stempl_, method_, smask_);
        
        for (auto i = 0; i != nms_count_; ++i) {
            // pixel-level roughly search
            cv::Point loc;
            double score;
            cv::minMaxLoc(match1, nullptr, &score, nullptr, &loc);
            log.debug("[Random_Based] #{}: roughly-search({}, {}, {})", i, score, loc.x, loc.y);
            cv::circle(match1, loc, nms_radius_, 0, -1);

            // pixel-level finely search
            {
                auto x = loc.x * s_;
                auto y = loc.y * s_;
                auto h = templ_.rows + (2 * s_);
                auto w = templ_.cols + (2 * s_);
                if (x + w >= image.cols || y + h >= image.rows) {
                    log.warn(   "{}:{}\n"
                                "**************************************\n"
                                "Rect range out of image size. Continue\n"
                                "**************************************\n", 
                                __FILE__, __LINE__);
                    continue;
                }
                auto patch = image(cv::Rect(x, y, w, h));
                auto match2 = chipimgproc::match_template(patch, templ_, method_, mask_);

                cv::Point dxy;
                cv::minMaxLoc(match2, nullptr, &score, nullptr, &dxy);
                log.debug("[Random_Based] #{}: finely-search({}, {}, {})", i, score, loc.x, loc.y);
                locations.push_back(cv::Point(dxy.x + x, dxy.y + y));

                // filter bad identifications out
                if (best_score >= score)
                    continue;
            }

            // evaluate transformation matrix
            std::vector<cv::Vec2f> anchors;
            auto x = locations.back().x;
            auto y = locations.back().y;
            auto w = templ_.cols;
            auto h = templ_.rows;
            auto view = image(cv::Rect(x, y, w, h));
            try {
                cv::Matx23f wmatx = cv::Matx23f::eye();
                cv::findTransformECC(
                    view, templ_, 
                    wmatx, cv::MOTION_EUCLIDEAN, 
                    criteria_, mask_
                );
                cv::invertAffineTransform(wmatx, wmatx);
                cv::transform(anchors_, anchors, wmatx);
            } catch (...) {
                continue;
            }

            // decoding
            auto [index, distance] = derived()->identify(view, anchors, FWD(identify_args)...);

            // set new template and anchors for decoding
            if( index >= 0) {
                best_score = score;
                new_templ = view;
                new_anchors = anchors;
            }
        }
        
        if(new_templ.empty()) {
            throw std::runtime_error("template decoding failed");
        }

        // identify marker locations with new template
        std::vector<std::tuple<int, double, cv::Point2d>> results;
        auto h = new_templ.rows;
        auto w = new_templ.cols;
        for(auto&& [x, y] : locations) {
    
            // subpixel-level search
            auto view = image(cv::Rect(x, y, w, h));
            cv::Matx23f wmatx = cv::Matx23f::eye();
            double score = 0.0;
            cv::Point2d center;
            try {
                // std::string str("(" + std::to_string(fov_id.x) + "," + std::to_string(fov_id.y) + ")-" + std::to_string(i) + "-(" + std::to_string(x) + "," + std::to_string(y) + ").tiff");
                // cv::imwrite("view" + str, view);
                // cv::imwrite("new_templ" + str, new_templ);
                // cv::imwrite("mask" + str, mask_);
                score = cv::findTransformECC(
                    view, new_templ, wmatx, 
                    cv::MOTION_TRANSLATION, criteria_, mask_
                );
                center.x = x + new_anchors.back()[0] - wmatx(0, 2);
                center.y = y + new_anchors.back()[1] - wmatx(1, 2);
            } catch(...) {
                continue;
            }
            // cv::imwrite("view.png", view);
            // decoding
            auto [index, distance] = derived()->identify(
                view, 
                new_anchors, 
                FWD(identify_args)...
            );

            // save result
            results.emplace_back(index, score, center);
            // ++i;
        }

        std::sort(results.begin(), results.end(), [](auto&& r0, auto&& r1){
            auto&& [i0, s0, c0] = r0;
            auto&& [i1, s1, c1] = r1;
            if(i0 != i1) return i0 > i1;
            if(s0 != s1) return s0 > s1;
            return false;
        });

        return results;
    }
protected:
    cv::Mat_<std::uint8_t>  templ_, stempl_ ; 
    cv::Mat_<std::uint8_t>  mask_, smask_   ; 
    std::int32_t            pyramid_level_  ;
    double                  theor_max_val_  ;
    std::int32_t            nms_count_      ;
    std::int32_t            nms_radius_     ;
    cv::TermCriteria        criteria_       ;
    std::vector<cv::Vec2f>  anchors_        ;
    std::int32_t            s_              ;
    cv::TemplateMatchModes  method_         ;
};

}
