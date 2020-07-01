/**
 * @file aruco_reg_mat.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::marker::detection::ArucoRegMat
 */
#pragma once
#include <vector>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/const.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/aruco.hpp>
#include "reg_mat.hpp"
#include "aruco_random.hpp"
namespace chipimgproc::marker::detection {

/**
 * @brief The regular matrix layout ArUco marker detection.
 * 
 */
class ArucoRegMat {
    auto generate_raw_marker_regions(
        const cv::Mat&          src, 
        double                  mk_invl_x,
        double                  mk_invl_y,
        double                  mk_width,
        double                  mk_height,
        int                     fov_mk_num_x,
        int                     fov_mk_num_y
    ) const {

        // marker layout width and height
        auto mk_mat_w = mk_invl_x * (fov_mk_num_x - 1) + mk_width  ; // fov marker num
        auto mk_mat_h = mk_invl_y * (fov_mk_num_y - 1) + mk_height ;

        // marker layout origin
        auto x_org = ( src.cols / 2 ) - ( mk_mat_w / 2 );
        auto y_org = ( src.rows / 2 ) - ( mk_mat_h / 2 );

        // cut points
        std::vector<std::uint32_t> cut_points_x;
        std::vector<std::uint32_t> cut_points_y;
        std::vector<MKRegion> marker_regions;
        {
            std::int32_t last_x = - mk_width;
            for( std::int32_t x = x_org; x <= (x_org + mk_mat_w); x += mk_invl_x ) {
                cut_points_x.push_back((last_x + mk_width + x) / 2);
                last_x = x;
            }
            cut_points_x.push_back((last_x + mk_width + src.cols) / 2);

            std::int32_t last_y = - mk_height;
            for( std::int32_t y = y_org; y <= (y_org + mk_mat_h); y += mk_invl_y ) {
                cut_points_y.push_back((last_y + mk_height + y) / 2);
                last_y = y;
            }
            cut_points_y.push_back((last_y + mk_height + src.rows) / 2);
        }
        std::size_t y_last_i = 0;
        for(std::size_t y_i = 1; y_i < cut_points_y.size(); y_i ++ ) {
            auto& y      = cut_points_y.at(y_i);
            auto& y_last = cut_points_y.at(y_last_i);
            std::size_t x_last_i = 0;
            for(std::size_t x_i = 1; x_i < cut_points_x.size(); x_i ++ ) {
                MKRegion marker_region;
                auto& x      = cut_points_x.at(x_i);
                auto& x_last = cut_points_x.at(x_last_i);
                marker_region.x      = x_last;
                marker_region.y      = y_last;
                marker_region.width  = x - x_last;
                marker_region.height = y - y_last;
                marker_region.x_i    = x_i - 1;
                marker_region.y_i    = y_i - 1;
                // marker_region.info(out);
                marker_regions.push_back(marker_region);
                x_last_i = x_i;
            }
            y_last_i = y_i;
        }
        return marker_regions;
    }
    auto resize(cv::Mat& img, std::int32_t pyramid_level) const {
        cv::Mat img_tmp; 
        for( int i = 0; i < pyramid_level; i ++ ) {
            cv::pyrDown(img, img_tmp);
            img = img_tmp.clone();
        }
        
    }
public:

    ArucoRegMat(
        aruco::ConstDictionaryPtr dict, 
        const nlohmann::json&     id_map, 
        ArucoRandom&&             detector
    ) 
    : dictionary_   (dict)
    , layout_       (id_map)
    , detector_     (std::move(detector))
    {
    }


    /**
     * @brief Call operator, detect ArUco marker as regular matrix.
     * 
     * @param src               The input image.
     * @param marker_width      Marker width in pixel scale.
     * @param marker_height     Marker height in pixel scale.
     * @param out               A logger for outputting the recognition process details.
     * @param v_bin             Deprecated. Debug image output.
     * @param v_search          Deprecated. Debug image output. 
     * @param v_marker          Deprecated. Debug image output. 
     * @return std::vector<MKRegion> 
     *                          The detected marker regions.
     */
    std::vector<MKRegion> operator()(
        const cv::Mat&              src, 
        int                         marker_width,
        int                         marker_height,
        std::ostream&               out        = nucleona::stream::null_out,
        const ViewerCallback&       v_bin      = nullptr,
        const ViewerCallback&       v_search   = nullptr,
        const ViewerCallback&       v_marker   = nullptr
    ) const {
        std::vector<MKRegion> regions;
        auto results = detector_(src, layout_.get_marker_indices());
        auto top = std::distance(
            results.begin()
          , std::find_if(results.begin(), results.end(), [](auto&& tuple) {
                return std::get<0>(tuple) == -1;
            })
        );
        for (auto k = 0; k < top; ++k) {
            auto&& [idx, score, loc] = results.at(k);
            auto pos = layout_.get_sub(idx);
            MKRegion mk;
            mk.x = std::round(loc.x - (marker_width  - 1) * 0.5);
            mk.y = std::round(loc.y - (marker_height - 1) * 0.5);
            mk.width = marker_width;
            mk.height = marker_height;
            mk.x_i = pos.x;
            mk.y_i = pos.y;
            mk.score = score;
            regions.push_back(std::move(mk));
        }
        return regions;
    }

    /**
     * @brief Call operator, detect ArUco marker as regular matrix.
     * 
     * @param src               The input image.
     * @param mk_layout         The marker layout of image.
     * @param unit              Image matrix unit.
     * @param out               A logger for outputting the recognition process details.
     * @param v_bin             Deprecated. Debug image output.
     * @param v_search          Deprecated. Debug image output. 
     * @param v_marker          Deprecated. Debug image output. 
     * @return std::vector<MKRegion> 
     *                          The detected marker regions.
     */
    std::vector<MKRegion> operator()(
        const cv::Mat&              src, 
        const Layout&               mk_layout, 
        const MatUnit&              unit,
        std::ostream&               out        = nucleona::stream::null_out,
        const ViewerCallback&       v_bin      = nullptr,
        const ViewerCallback&       v_search   = nullptr,
        const ViewerCallback&       v_marker   = nullptr
    ) const {
        auto& mk_des = mk_layout.get_single_pat_marker_des();
        auto& marker = mk_des.get_std_mk(unit);
        return operator()(
            src, marker.cols, marker.rows, out,
            v_bin, v_search, v_marker
        );
    }

private:
    aruco::ConstDictionaryPtr               dictionary_             ;
    aruco::MarkerMap                        layout_                 ;
    ArucoRandom                             detector_               ;
};

struct MakeArucoRegMat {
    /**
     * @brief Set the internal ArUco detector parameters.
     * 
     * @param pyramid_level         A number of downsampling levels for speeding up marker localization.
     *                              The value is an integer > 0. Experientially, setting the value of levels to 2 or 3 is good enough.
     * @param border_bits           A number of bits for border edge generation.
     *                              Please refer to the symbol b in the schematic diagram.
     * @param fringe_bits           A number of bits for fringe edge generation.
     *                              Please refer to the symbol f in schematic diagram.
     * @param a_bit_width           The width of a bit pattern in pixel scales. This value needs to be estimated precisely. 
     *                              Please refer to the symbol p in schematic diagram.
     * @param margin_size           The width of the margin for matching frame template.
     *                              This value must be greater than 0 and less than a size of border width.
     *                              Experientially, setting the parameter to border_bits x 0.6 is a practical.
     *                              The value unit is pixel.
     *                              Please refer to the symbol m in schematic diagram.
     * @param frame_template_path   The template image file path for marker localization.
     * @param frame_mask_path       The mask image file path for matching frame template.
     *                              It is used to exclude the patterns of coding region in matching process.
     * @param nms_count             An upper bound to the number of markers in an observed image.
     * @param nms_radius            A lower bound to the distance between ArUco markers in pixel scales.
     * @param cell_size             The size of the interior region for a bit detection.
     *                              Please refer to the symbol s in schematic diagram.
     * @param ids                   A list of candidate marker IDs possibly detected in an image.
     * @param mk_index_map          A mapper from ArUco marker ID to marker location ID.
     * 
     * @param logger                A logger for outputting the recognition process details.
     */
    auto operator()(
        aruco::ConstDictionaryPtr       dict,
        const nlohmann::json&           id_map,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const std::int32_t&             nms_count,
        const double&                   nms_radius,
        const double&                   ext_width
    ) const {
        return ArucoRegMat(
            dict, 
            id_map, 
            make_aruco_random(
                dict,
                templ,
                mask,
                aruco_width,
                pyramid_level,
                nms_count,
                nms_radius,
                ext_width
            )
        );
    }
    auto operator()(
        aruco::Dictionary&&             dict,
        const nlohmann::json&           id_map,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const std::int32_t&             nms_count,
        const double&                   nms_radius,
        const double&                   ext_width
    ) const {
        aruco::ConstDictionaryPtr ptr(new aruco::Dictionary(std::move(dict)));
        return this->operator()(
            ptr, id_map, templ, mask, aruco_width, 
            pyramid_level, nms_count, nms_radius, ext_width
        );
    }
    auto operator()(
        const std::string&              db_path,
        const std::string&              db_key,
        const nlohmann::json&           id_map,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const std::int32_t&             nms_count,
        const double&                   nms_radius,
        const double&                   ext_width
    ) const {
        return operator()(
            aruco::Dictionary::from_db_and_key(db_path, db_key),
            id_map,
            templ,
            mask,
            aruco_width,
            pyramid_level,
            nms_count,
            nms_radius,
            ext_width
        );

    }
};

constexpr MakeArucoRegMat make_aruco_reg_mat;
}