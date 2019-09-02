/**
 *  @file    ChipImgProc/aruco/detector.hpp
 *  @author  Chia-Hua Chang, Alex Lee
 *  @brief   Detect ArUco markers in an image.
 *  @details The ArUco marker detection algorithm
 * 
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
#include <ChipImgProc/logger.hpp>
#include "mk_img_dict.hpp"
#include <ChipImgProc/algo/fixed_capacity_set.hpp>
#include <ChipImgProc/utils/pos_comp_by_score.hpp>

namespace chipimgproc::aruco {

/**
 *  @brief The ArUco marker detector class.
 *  @details Here shows an example
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
     * @brief  Reset the detector parameters
     * @param  dict            Dictionary for ArUco coding binaries.
     * @param  pyramid_level   A number of downsampling levels for speeding up marker localization.
     *                         The value is an integer > 0. Experientially, setting the value of levels to 2 or 3 is good enough.
     * @param  border_bits     A number of bits for border edge generation.
     *                         Please refer to the symbol b in the schematic diagram.
     * @param  fringe_bits     A number of bits for fringe edge generation.
     *                         Please refer to the symbol f in schematic diagram.
     * @param  a_bit_width     The width of a bit pattern in pixel scales. This value needs to be estimated precisely. 
     *                         Please refer to the symbol p in schematic diagram.
     * @param  margin_size     The width of the margin for matching frame template.
     *                         This value must be greater than 0 and less than a size of border width.
     *                         Experientially, setting the parameter to border_bits x 0.6 is a practical.
     *                         Please refer to the symbol m in schematic diagram.
     * @param  frame_template  The template image for marker localization.
     *                         It should be determined by border_bits, fringe_bits, a_bit_width and margin_size parameters.
     * @param  frame_mask      The mask for matching frame template.
     *                         It is used to exclude the patterns of coding region in matching process.
     *                         The image should be determined by border_bits, fringe_bits, a_bit_width and margin_size parameters.
     * @param  nms_count       An upper bound to the number of markers in an observed image.
     * @param  nms_radius      A lower bound to the distance between ArUco markers in pixel scales.
     * @param  cell_size       The size of the interior region for a bit detection.
     *                         Please refer to the symbol s in schematic diagram.
     * @param  ids             A list of candidate marker IDs possibly detected in an image.
     * @param  logger          A logger for outputting the recognition process details.
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

        auto side_bits_length = coding_bits_ + border_bits_ * 2 + fringe_bits_ * 2;
        auto length = margin_size_ * 2;
        length += a_bit_width_ * side_bits_length;

        cv::resize(templ_, templ_, cv::Size(length, length));
        cv::resize(mask_ , mask_ , cv::Size(length, length));
        if(pyramid_level_ > 0) {
            cv::pyrDown(templ_, small_templ_);
            cv::pyrDown(mask_ , small_mask_ );
        }
        for (auto i = 1; i < pyramid_level_; ++i) {
            cv::pyrDown(small_templ_, small_templ_);
            cv::pyrDown(small_mask_ , small_mask_ );
        }

        marker_img_dict_.reset(
            dict, ids, a_bit_width, 
            coding_bits_,
            pyramid_level
        );

    }
    auto aruco_img(std::int32_t mk_index) const {
        return marker_img_dict_.mk_idx_at(mk_index, small_templ_);
    }
    /**
     *  @brief   Detect markers in an image
     *  @param   input   Input image
     *  @param   logger  Logger for outputting processing details.
     *  @return  A list of marker ids and corresponding locations in pixel scales
     */
    auto detect_markers(
        cv::Mat input,
        std::ostream& logger = nucleona::stream::null_out
    ) const {

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
        if(pyramid_level_ > 0) {
            cv::pyrDown(image, small_image);
        }
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
        for (auto k = 0; k < nms_count_; ++k) {

            // coarse alignment + non-maximum suppression
            cv::Point loc;
            cv::minMaxLoc(scores, nullptr, nullptr, nullptr, &loc);
            cv::circle(scores, loc + offset, nms_radius_ / scale, cv::Scalar(0.0), -1);
            loc.x = std::round(scale * loc.x);
            loc.y = std::round(scale * loc.y);
            if (loc.x > image.cols - templ_.cols)
                loc.x = image.cols - templ_.cols;
            if (loc.y > image.rows - templ_.rows)
                loc.y = image.rows - templ_.rows;

            // fine alignment
            cv::Mat_<uint8_t> view = image(cv::Rect(loc, templ_.size()));
            cv::Matx<float,2,3> warp_mat = cv::Matx<float,2,3>::eye();
            double score = -1;
            try{
                // cv::imwrite("view.tiff", view);
                score = cv::findTransformECC(view, templ_, warp_mat, cv::MOTION_EUCLIDEAN, criteria, mask_);
            } catch(const cv::Exception& e) {
                logger << "ECC convergence failure. skip this one\n";
                continue;
            }
            chipimgproc::log.debug("ECC score: {}", score);
            // logger << "ECC score: " << score << '\n';

            // transform the anchor points
            std::vector<cv::Vec<double,2>> dst_points;
            try {
                cv::invertAffineTransform(warp_mat, warp_mat);
                cv::transform(src_points, dst_points, warp_mat);
            } catch (const cv::Exception& e) {
                logger << "invert affine transformation failed. skip this one\n";
                continue;
            }

            // binarization
            cv::Mat_<uint8_t> bw;
            cv::threshold(view, bw, 0, 1, cv::THRESH_BINARY | cv::THRESH_OTSU);

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
            int32_t i, j;
            cv::Rect region(cv::Point(0, 0), cell_size_);
            try {
                for (j = first; j < last; ++j) {
                    auto r = static_cast<double>(j) / (total - 1);
                    for (i = first; i < last; ++i) {
                        auto c = static_cast<double>(i) / (total - 1);
                        auto p = (dst_points[0] * (1 - c) + dst_points[1] * c) * (1 - r)
                               + (dst_points[2] * (1 - c) + dst_points[3] * c) * (    r) ;
                        region.x = std::round(p[0] - 0.5 * cell_size_.width );
                        region.y = std::round(p[1] - 0.5 * cell_size_.height);

                        // if (region.x > bw.cols - cell_size_.width) {
                        //     logger().debug("x boundary handling at line %d", __LINE__);
                        //     region.x = bw.cols - cell_size_.width;
                        // } else if (region.x < 0) {
                        //     logger().debug("x boundary handling at line %d", __LINE__);
                        //     region.x = 0;
                        // }
                        // if (region.y > bw.rows - cell_size_.height) {
                        //     logger().debug("y boundary handling at line %d", __LINE__);
                        //     region.y = bw.rows - cell_size_.height;
                        // } else if (region.y < 0) {
                        //     logger().debug("y boundary handling at line %d", __LINE__);
                        //     region.y = 0;
                        // }

                        // std::cerr << static_cast<int32_t>(norm(bw(region), cv::NORM_L1)) << " ";
                        uint64_t bit = norm(bw(region), cv::NORM_L1) > cell_size_.area() * 0.5;
                        code |= bit << shift++;

                        // GUI verbose
                        // cv::rectangle(plane, region, cv::Scalar(255, 255, 255), 1);
                    }
                    // std::cerr << "| ";
                }
            } catch (const cv::Exception& e) {
                logger << "binary code extraction failed, skip this one\n";
                std::stringstream ss;
                ss << "region: " << region << " / " << "bwsize: " << bw.size();
                logger << "bit: " <<  i + 1 << ',' << j + 1 << '\n';
                logger << ss.str() << '\n';
                continue;
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
    MkImgDict                   marker_img_dict_     ; // marker index => image
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
