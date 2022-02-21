/**
 * @file    fusion_array.hpp
 * @author  Chi-Hsuan Ho (jeffho@centrilliontech.com.tw)
 * @brief   @copybrief chipimgproc::marker::detection::FusionArray
 */
#pragma once

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/video/tracking.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/const.h>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/algo/fixed_capacity_set.hpp>
#include <ChipImgProc/utils/pos_comp_by_score.hpp>
#include <ChipImgProc/logger.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <Nucleona/tuple.hpp>
#include <Nucleona/range.hpp>
#include <Nucleona/language.hpp>
#include <functional>

#include <iostream>

namespace chipimgproc::marker::detection {
namespace cm = chipimgproc;
using cvMat8 = cv::Mat_<std::uint8_t>;

struct MakeFusionArray;
/**
 * @brief The FussionArray class is used to construct the detection technique for general markers.
 * 
 */
template<class cvMatT>
struct FusionArray {

friend MakeFusionArray;
protected:
    /**
     * @brief     The detection technique for general markers distributed in a 
     *            rectangular arrangement only (it will support for other arrangements
     *            in the future).
     * @details   This algorithm first use the chip GDS spec to generate the candidate 
     *            marker regions to narrow down the searching area. From each candidate 
     *            marker region of the corresponding alignment marker, use the marker 
     *            frame template (templ) and marker frame mask (mask) to Identify the 
     *            pixel-level marker position. To improve the efficiency, this algorithm 
     *            first detect marker positions in the downsampled images, and use those 
     *            rough positions to find the accurate position in the original scale image.
     *            Finally, it will store all the information (row ID and column ID of the 
     *            alignment markers, matching score, detected marker position) into a 
     *            collection.
     *            
     *            For more information, please refer to the source code.
     *            
     *            Examples:
     * 
     *            Summit.Grid: <a href="http://gitlab.centrilliontech.com.tw:10088/centrillion/Summit.Grid/blob/1.3.x/include/summit/app/grid/probe_ch_proc_setter.hpp#L56">include/summit/app/grid/probe_ch_proc_setter.hpp: 56</a>
     * 
     * @image     html FusionArray-concept.png width=650px
     * 
     * @param templ             Template image that is used to recognize the marker.
     * @param mask              Mask image that is used to inform the region that should be focused.
     * @param pyramid_level     The downsampling times. This parameter is used to downsample the processed 
     *                          input image, templ and mask.
     * @param theor_max_val     Theoretical maximum value of the input image depth. This parameter is 
     *                          only be used to convert the image depth of the processed input image.
     * @param img_templ_cols    Number of column in the template image.
     * @param img_templ_rows    Number of row in the template image.
     * @param mk_layout         Marker layout generated from the chipimgproc::marker::Layout class.
     * @param unit              Unit specifying the current working environment for input images, the algorithm, and the output results unit (PX, CELL).
     */
    FusionArray(
        const cv::Mat_<cvMatT>&    templ,
        const cv::Mat_<cvMatT>&    mask,
        const std::int32_t&        pyramid_level,
        const double&              theor_max_val,
        const int&                 img_templ_cols,
        const int&                 img_templ_rows,
        const Layout&              mk_layout, 
        const MatUnit&             unit
    )
    : templ_             (templ)
    , stempl_            ()
    , mask_              (mask)
    , smask_             ()
    , marker_regions_    ()
    , pyramid_level_     (pyramid_level)
    , theor_max_val_     (theor_max_val)
    , criteria_          (
        cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 300, 1e-5
    )
    , anchors_           ()
    , s_                 (1 << pyramid_level)
    , method_            (cv::TM_CCORR_NORMED)
    {
        assert(templ.size() == mask.size());
        auto dsize_w = templ.cols;
        auto dsize_h = templ.rows;

        // Prepare the downsampled templates and masks for the template matching.
        for(auto i = pyramid_level_; i > 0; i --){
            dsize_w = (dsize_w + 1) / 2;
            dsize_h = (dsize_h + 1) / 2;
        }
        if(pyramid_level > 0){
            cv::resize(templ_, stempl_, cv::Size(dsize_w, dsize_h), 0, 0, cv::INTER_AREA);
            cv::resize(mask_,  smask_,  cv::Size(dsize_w, dsize_h), 0, 0, cv::INTER_NEAREST);
        } else {
            stempl_ = templ ;
            smask_  = mask  ;
        }

        // Generate the marker regions.
        marker_regions_ = generate_raw_marker_regions(img_templ_cols,
                                                      img_templ_rows, 
                                                      mk_layout, 
                                                      unit,
                                                      std::cout);

    }

public:
    /**
     * @brief Generation of the inference for the area where markers could appear accroding to the regular scanning process. 
     *        This function is the same as that in the chipimgproc::marker::detection::RegMat Class.
     * 
     * @param img_templ_cols    Number of column in the template image.
     * @param img_templ_rows    Number of row in the template image.
     * @param mk_layout         Marker layout generated from the chipimgproc::marker::Layout class.
     * @param unit              Unit specifying the current working environment for input images, the algorithm, and the output results unit (PX, CELL).
     */
    auto generate_raw_marker_regions(
        const int&                   img_templ_cols,
        const int&                   img_templ_rows,
        const Layout&                mk_layout,
        const MatUnit&               unit,
        std::ostream&                out
    ) const {
        /* Unit: pixel/cell */

        // Distance between two adjacent markers in a chip.
        auto [mk_invl_x, mk_invl_y] = mk_layout.get_marker_invl(unit);  // chip.json

        // Width and height of a marker.
        auto mk_width  = mk_layout.get_marker_width (unit) ;  // chip.json
        auto mk_height = mk_layout.get_marker_height(unit) ;  // chip.json

        // Width and height of the marker layout.
        auto mk_mat_w = mk_invl_x * (mk_layout.mk_map.cols - 1) + mk_width  ; // fov marker num
        auto mk_mat_h = mk_invl_y * (mk_layout.mk_map.rows - 1) + mk_height ;

        // Theoretical origin of the marker layout in the image.
        auto x_org = ( img_templ_cols / 2 ) - ( mk_mat_w / 2 );
        auto y_org = ( img_templ_rows / 2 ) - ( mk_mat_h / 2 );

        // Cut points
        std::vector<std::uint32_t> cut_points_x;
        std::vector<std::uint32_t> cut_points_y;
        std::vector<MKRegion> marker_regions;
        {
            std::int32_t last_x = - mk_width;
            for( std::int32_t x = x_org; x <= (x_org + mk_mat_w); x += mk_invl_x ) {
                cut_points_x.push_back((last_x + mk_width + x) / 2);
                last_x = x;
            }
            cut_points_x.push_back((last_x + mk_width + img_templ_cols) / 2);

            std::int32_t last_y = - mk_height;
            for( std::int32_t y = y_org; y <= (y_org + mk_mat_h); y += mk_invl_y) {
                cut_points_y.push_back((last_y + mk_height + y) / 2);
                last_y = y;
            }
            cut_points_y.push_back((last_y + mk_height + img_templ_rows) / 2);
        }
        std::size_t y_last_i = 0;
        for(std::size_t y_i = 1; y_i < cut_points_y.size(); y_i ++ ) {
            auto& y      = cut_points_y.at(y_i);
            auto& y_last = cut_points_y.at(y_last_i);
            std::size_t x_last_i = 0;
            for(std::size_t x_i = 1; x_i < cut_points_x.size(); x_i ++) {
                MKRegion marker_region;
                auto& x      = cut_points_x.at(x_i);
                auto& x_last = cut_points_x.at(x_last_i);
                marker_region.x      = x_last;
                marker_region.y      = y_last;
                marker_region.width  = x - x_last;
                marker_region.height = y - y_last;
                marker_region.x_i    = x_i - 1;
                marker_region.y_i    = y_i - 1;
                marker_regions.push_back(marker_region);
                x_last_i = x_i;
            }
            y_last_i = y_i;
        }
        return marker_regions;
    }
    /**
     * @brief Set the preprocessor of the fluorescent channel images.
     * 
     * @return void 
     */
    void set_pb_img_preprocessor(void) {
        img_preprocessor_ = [](const cvMat8& mat) -> cvMat8 {
            cvMat8 img;
            img = norm_u8(mat);
            return img;
        };
    }
    /**
     * @brief Set the preprocessor of the white channel images.
     * 
     * @return void 
     */    
    void set_wh_img_preprocessor(void) {
        img_preprocessor_ = [](const cvMat8& mat) -> cvMat8 {
            cvMat8 img;
            img = cv::max(mat, cv::mean(mat)[0]);
            return img;
        };
    }
    /**
     * @brief     This function perform the FusionArray detection technique algorithm 
     *            to recognize the positions of each general marker.
     * @param     input Input images with general markers.
     * @return    A collection of detected marker ID with their matching scores and
     *            xy-positions in the corresponding images.
     */
    std::vector<
        std::tuple<cv::Point, double, cv::Point2d>
    > operator() (
        cv::Mat input
    ) const {
        // Convert input image to CV_8U
        cvMat8 image;
        if (input.depth() == CV_8U)
            input.copyTo(image);
        else if (input.depth() == CV_16U)
            input.convertTo(image, CV_8U, 255.0 / theor_max_val_);
        else 
            throw std::invalid_argument("Invalid input image format detected.");

        // Preprocess input images.        
        image = img_preprocessor_(image);
        // cv::imwrite("processed_fluo_image.tiff", image);

        // int t = 0;
        std::vector<std::tuple<cv::Point, double, cv::Point2d>> results;
        // Divide the image into subarea.
        for(auto& mk_r : marker_regions_) {
            cv::Mat target = image(mk_r);
            // Pyramid downsampling.
            cvMat8 starget = target.clone();
            for (auto i = 0; i < this->pyramid_level_; ++i)
                cv::pyrDown(starget, starget);
            {
                auto tmp = starget(cv::Rect(1, 1, starget.cols - 2, starget.rows - 2));
                starget = tmp;
            }

            // Search all possible marker locations (template matching on downsampling domain).
            auto match1 = cm::match_template(starget, stempl_, method_, smask_);
            cv::Point loc;
            cv::minMaxLoc(match1, nullptr, nullptr, nullptr, &loc);

            // Search all possible marker locations (pixel-level finely search).
            cv::Point mk_id(mk_r.x_i, mk_r.y_i);
            double score;
            cv::Point2d mk_loc_center;
            {
                double map_buffer_r = 2.0; // 1.0 stands for no buffer area.
                auto w = templ_.cols + (2 * s_);
                auto h = templ_.rows + (2 * s_);
                auto x = loc.x * s_ - std::round((map_buffer_r / 2 - 0.5) * w);
                auto y = loc.y * s_ - std::round((map_buffer_r / 2 - 0.5) * h);

                cvMat8 patch;
                bool top_out, bottom_out, left_out, right_out;
                left_out   = mk_r.x + x < 0;
                top_out    = mk_r.y + y < 0;
                right_out  = mk_r.x + x + map_buffer_r * w >= image.cols;
                bottom_out = mk_r.y + y + map_buffer_r * h >= image.rows;
                if (top_out || bottom_out || left_out || right_out) {
                    log.warn("ROI out of image range. Use the bilinear interpolation version ROI.");
                    cv::Point2d center(mk_r.x + x + (map_buffer_r * w - 1) / 2.0, mk_r.y + y + (map_buffer_r * h - 1) / 2.0);
                    cv::getRectSubPix(image, cv::Size2d(map_buffer_r * w, map_buffer_r * h), center, patch);
                } else {
                    patch = image(cv::Rect(mk_r.x + x, mk_r.y + y, map_buffer_r * w, map_buffer_r * h));
                }

                auto match2 = cm::match_template(patch, templ_, method_, mask_);

                cv::Point dxy;
                cv::minMaxLoc(match2, nullptr, &score, nullptr, &dxy);
                h = templ_.rows;
                w = templ_.cols;
                mk_loc_center = cv::Point2d(mk_r.x + x + dxy.x + ((w - 1) / 2.0), mk_r.y + y + dxy.y + ((h - 1) / 2.0));

                // std::cout << "(" << mk_loc_center.x << ", " << mk_loc_center.y << ")" << std::endl;
            }

            // Save results
            results.emplace_back(mk_id, score, mk_loc_center);
            // ++t;
        }

        // Calculate the corresponding score result.

        // TODO: score_processor - Filter the low score or unreasonable score.
        // TODO: position check by circular mask.
        // TODO: get different number points in different regions by using MKRegion data structure.
        // TODO: region generator for circular region.
        // TODO: Uni test for fluorescent image gridding processing.
        // TODO: wh_img_preprocessor.
        // TODO: subpixel-level search (Further decision).
        // TODO - Outside this class: circular region filter.

        return results;
    }

protected:
    cv::Mat_<cvMatT>                          templ_, stempl_   ;
    cv::Mat_<cvMatT>                          mask_,   smask_   ;
    cv::TermCriteria                          criteria_         ;
    cv::TemplateMatchModes                    method_           ;
    std::vector<MKRegion>                     marker_regions_   ;
    std::vector<cv::Vec2f>                    anchors_          ;
    std::int32_t                              pyramid_level_    ;
    std::int32_t                              s_                ;
    std::function<cvMat8(const cvMat8&)>      img_preprocessor_ ;
    double                                    theor_max_val_    ;
};

struct MakeFusionArray {

    auto operator()(
        const cvMat8&                         templ,
        const cvMat8&                         mask,
        const std::int32_t&                   pyramid_level,
        const double                          theor_max_val,
        const int&                            img_templ_cols,
        const int&                            img_templ_rows,
        const Layout&                         mk_layout,
        const MatUnit&                        unit
    ) const {
        return FusionArray<std::uint8_t>(
            templ,
            mask,
            pyramid_level,
            theor_max_val,
            img_templ_cols,
            img_templ_rows,
            mk_layout,
            unit
        );
    }
    auto operator()(
        const cvMat8&                         templ,
        const cvMat8&                         mask,
        const std::int32_t&                   pyramid_level,
        const double&                         theor_max_val,
        const cv::Mat&                        img_template,
        const Layout&                         mk_layout,
        const MatUnit&                        unit
    ) const {
        return this->operator()(
            templ,
            mask,
            pyramid_level,
            theor_max_val,
            img_template.cols,
            img_template.rows,
            mk_layout,
            unit
        );
    }
    auto operator()(
        const cvMat8&                         templ,
        const cvMat8&                         mask,
        const std::int32_t&                   pyramid_level,
        const double&                         theor_max_val,
        const cv::Mat&                        img_template,
        const Layout&                         mk_layout
    ) const {
        return this->operator()(
            templ,
            mask,
            pyramid_level,
            theor_max_val,
            img_template,
            mk_layout,
            cm::MatUnit::PX
        );
    }
};
constexpr MakeFusionArray make_fusion_array;

}