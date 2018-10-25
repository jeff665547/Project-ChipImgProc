/**
 *  @file    ChipImgProc/aruco/detector.hpp
 *  @author  Chia-Hua Chang, Alex Lee
 *  @brief   Detect ArUco marker in a image.
 */
#pragma once
#include <cstdint>
#include <vector>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <optional>
#include <ChipImgProc/utils.h>
#include <cinttypes>
#include "utils.hpp"
#include "dictionary.hpp"
#include <Nucleona/stream/null_buffer.hpp>

namespace chipimgproc::aruco {

/**
 *  @brief The ArUco marker detector class.
 *  @details Heres a simple usage example
 *  @snippet ChipImgProc/aruco_test.cpp usage
 */
class Detector {
  public:
    Detector()
    : dictionary_         (nullptr)
    , pyramid_level_      ()
    , coding_bits_        ()
    , border_bits_        ()
    , fringe_bits_        ()
    , maxcor_bits_        ()
    , a_bit_width_        ()
    , margin_size_        ()
    , templ_              ()
    , small_templ_        ()
    , mask_               () 
    , small_mask_         ()
    , nms_count_          ()
    , nms_radius_         ()
    , cell_size_          ()
    , ids_                ()
    {}
    /**
     *  @brief  Reset the meta parameter of ArUco marker detector.
     *  @param  dict            Dictionary file
     *  @param  pyramid_level   A downsampling level for marker localization.
     *  @param  border_bits     The thickness of ArUco code boarder in bit units.
     *  @param  fringe_bits     The thickness of chip chromium width in bit units.
     *  @param  a_bit_width     A ArUco bit in image pixel width.
     *  @param  margin_size     The margin size in pixel of template image.
     *  @param  frame_template  The template image for marker localization.
     *  @param  frame_mask      The mask for template search.
     *  @param  nms_count       The max count of markers in next detected image.
     *  @param  nms_radius      The min distance between detected markers.
     *  @param  cell_size       The ROI size of a grid cell.
     *  @param  ids             The candidate ArUco code ids in dictionary.
     *  @param  logger          Logger for output process detail.
     */
    void reset(
        const Dictionary&                 dict
      , const std::int32_t&               pyramid_level
      , const std::int32_t&               border_bits               // bits
      , const std::int32_t&               fringe_bits               // bits
      , const double&                     a_bit_width               // pixels
      , const double&                     margin_size               // pixels 
      , const cv::Mat&                    frame_template
      , const cv::Mat&                    frame_mask
      , const std::int32_t&               nms_count
      , const double&                     nms_radius                // pixels
      , const std::int32_t&               cell_size                 // pixels
      , const std::vector<std::int32_t>&  ids
      , std::ostream&                     logger            = nucleona::stream::null_out
    ) {

        logger << "pyramid_level          :" << pyramid_level          << '\n'; 
        logger << "border_bits            :" << border_bits            << '\n';   
        logger << "fringe_bits            :" << fringe_bits            << '\n';   
        logger << "a_bit_width            :" << a_bit_width            << '\n';   
        logger << "margin_size            :" << margin_size            << '\n';   
        logger << "nms_count              :" << nms_count              << '\n'; 
        logger << "nms_radius             :" << nms_radius             << '\n';   
        logger << "cell_size              :" << cell_size              << '\n';   

        chipimgproc::info(logger, frame_template   );
        chipimgproc::info(logger, frame_mask       );

        dictionary_     = &dict;
        pyramid_level_  = pyramid_level;
        a_bit_width_    = a_bit_width;
        coding_bits_    = dictionary_->coding_bits();
        border_bits_    = border_bits;
        fringe_bits_    = fringe_bits;
        maxcor_bits_    = dictionary_->maxcor_bits();
        margin_size_    = margin_size;
        templ_          = frame_template;
        mask_           = frame_mask;
        nms_count_      = nms_count;
        nms_radius_     = nms_radius;
        cell_size_      = cv::Size( cell_size, cell_size );
        ids_            = ids;

        auto length = margin_size_ * 2;
        length += a_bit_width_ * (coding_bits_ + border_bits_ * 2 + fringe_bits_ * 2);

        cv::resize(templ_, templ_, cv::Size(length, length));
        cv::resize(mask_ , mask_ , cv::Size(length, length));
        cv::pyrDown(templ_, small_templ_);
        cv::pyrDown(mask_ , small_mask_ );
        for (auto i = 1; i < pyramid_level_; ++i) {
            cv::pyrDown(small_templ_, small_templ_);
            cv::pyrDown(small_mask_ , small_mask_ );
        }
    }
    /**
     *  @brief   Detect  ArUco markers in a image
     *  @param   input   Target image
     *  @param   logger  Logger for output process detail.
     *  @return  A list of identifeid dictionary ids and points in pixel
     */
    auto detect_markers(
        cv::Mat input,
        std::ostream& logger = nucleona::stream::null_out
    ) {
        
        // convert to 8U
        cv::Mat_<uint8_t> image;
        if (input.depth() == CV_8U)
            input.copyTo(image);
        else if (input.depth() == CV_16U)
            input.convertTo(image, CV_8U, 255.0 / 16383.0);
        else // invalid input data
            throw std::invalid_argument("Invalid input format");
            
        // do pyramid downsampling
        cv::Mat_<uint8_t> small_image;
        cv::pyrDown(image, small_image);
        for (auto i = 1; i < pyramid_level_; ++i)
            cv::pyrDown(small_image, small_image);
        
        // define contants
        cv::TermCriteria criteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 500, 1e-3);
        cv::Point offset(small_templ_.cols / 2, small_templ_.rows / 2);
        const double scale = static_cast<double>(image.cols) / small_image.cols;
        const double width = a_bit_width_ * fringe_bits_ * 0.5 + margin_size_;
        std::vector<cv::Vec<double,2>> src_points { 
            {                   width,                   width }
          , { templ_.cols - 1 - width,                   width }
          , {                   width, templ_.rows - 1 - width }
          , { templ_.cols - 1 - width, templ_.rows - 1 - width }
        };

        // detect markers
        std::vector<std::tuple<int32_t,cv::Point_<float>>> results;
        cv::Mat_<float> scores(
            small_image.rows - small_templ_.rows + 1
          , small_image.cols - small_templ_.cols + 1
        );
        cv::matchTemplate(small_image, small_templ_, scores, cv::TM_CCORR_NORMED, small_mask_);
        logger << "nms_count_: " << nms_count_ << '\n';
        for (auto k = 0; k < nms_count_; ++k) {

            // coarse alignment + non-maximum suppression
            cv::Point loc;
            cv::minMaxLoc(scores, nullptr, nullptr, nullptr, &loc);
            cv::circle(scores, loc + offset, nms_radius_ / scale, cv::Scalar(0.0), -1);
            loc.x = std::round(scale * loc.x);
            loc.y = std::round(scale * loc.y);

            // fine alignment
            cv::Mat_<uint8_t> view = image(cv::Rect(loc, templ_.size()));
            cv::Matx<float,2,3> warp_mat = cv::Matx<float,2,3>::eye();
            double score = -1;
            try{
                score = cv::findTransformECC(view, templ_, warp_mat, cv::MOTION_EUCLIDEAN, criteria, mask_);
            } catch( const cv::Exception& e) {
                continue;
            }

            // transform the anchor points
            std::vector<cv::Vec<double,2>> dst_points;
            cv::invertAffineTransform(warp_mat, warp_mat);
            cv::transform(src_points, dst_points, warp_mat);

            // binarization
            cv::Mat_<uint8_t> bw;
            cv::threshold(view, bw, 128, 1, cv::THRESH_BINARY | cv::THRESH_OTSU);

            // GUI verbose
            // cv::Mat plane, planes[] {
            //     cv::Mat_<uint8_t>::zeros(view.size())
            //   , view
            //   , bw * 128
            // };
            // cv::merge(planes, 3, plane);

            // extract binary code
            auto total = coding_bits_ + (fringe_bits_ + border_bits_) * 2;
            auto first = fringe_bits_ + border_bits_;
            auto last  = total - first;
            auto code  = 0ull;
            auto shift = 0;
            cv::Rect region(cv::Point(0, 0), cell_size_);
            for (auto j = first; j < last; ++j) {
                auto r = static_cast<double>(j) / (total - 1);
                for (auto i = first; i < last; ++i) {
                    auto c = static_cast<double>(i) / (total - 1);
                    auto p = (dst_points[0] * (1 - c) + dst_points[1] * c) * (1 - r)
                           + (dst_points[2] * (1 - c) + dst_points[3] * c) * (    r) ;
                    region.x = std::round(p[0]) - cell_size_.width  / 2;
                    region.y = std::round(p[1]) - cell_size_.height / 2;
                    // std::cerr << static_cast<int32_t>(norm(bw(region), cv::NORM_L1)) << " ";
                    uint64_t bit = norm(bw(region), cv::NORM_L1) > cell_size_.area() * 0.5;
                    code |= bit << shift++;

                    // GUI verbose
                    // cv::rectangle(plane, region, cv::Scalar(255, 255, 255), 1);
                }
                // std::cerr << "| ";
            }

            // identify marker id with error bit correction
            int32_t index;
            if (dictionary_->identify(code, index, ids_, maxcor_bits_)) {
                // infer center point
                auto center_point = dst_points[0] * 0.25;
                for (auto i = 1; i != 4; ++i)
                    center_point += 0.25 * dst_points[i];
                center_point[0] += loc.x;
                center_point[1] += loc.y;

                // save results
                results.emplace_back(index, cv::Point_<float>(center_point));

                // verbose
                // auto d = bit_count(code ^ dictionary_[index]);
                // std::cerr << ": " << index << " (" << d << ")\n";
            } else {
                // std::cerr << ": marker not found" << '\n';
            }

            // GUI verbose
            // cv::imshow("Patch", decode(code, coding_bits_) * 255);
            // cv::imshow("Figure", plane);
            // cv::waitKey(0);
        }

        return results;
    }
  private:
    const Dictionary*           dictionary_          ;
    std::int32_t                pyramid_level_       ; // interger > 0
    std::int32_t                coding_bits_         ;
    std::int32_t                border_bits_         ;
    std::int32_t                fringe_bits_         ;
    std::int32_t                maxcor_bits_         ;
    double                      a_bit_width_         ;
    double                      margin_size_         ;
    cv::Mat_<std::uint8_t>      templ_, small_templ_ ;
    cv::Mat_<std::uint8_t>      mask_ , small_mask_  ;
    std::int32_t                nms_count_           ;
    double                      nms_radius_          ; // non-maximum suppression
    cv::Size                    cell_size_           ;
    std::vector<std::int32_t>   ids_                 ;
};

} // namespace aruco
