/**
 * @file aruco_reg_mat.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::marker::detection::ArucoRegMat
 */
#pragma once
#include <vector>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/const.h>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/aruco.hpp>
#include "reg_mat.hpp"
namespace chipimgproc::marker::detection {
/**
 * @brief The regular matrix layout ArUco marker detection.
 * 
 */
class ArucoRegMat 
{
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
    /**
     * @brief Set the ArUco marker dictionary.
     * @param db    The JSON format ArUco database. The database
     * @param key   The ArUco marker set id use to set the dictionary, for example: DICT_6X6_250.
     */
    void set_dict(const nlohmann::json& db, const std::string& key) {
        auto& dict_json = db[key];
        mk_index_map_.resize(dict_json["bitmap_list"].size());
        auto dict = aruco::Dictionary::from_json(dict_json);
        set_dict_(std::move(dict));
    }
    /**
     * @brief Equivlent to set_dict(const nlohmann::json& db, const std::string& key)
     *        but use JSON file path as the ArUco database source
     * 
     * @param path ArUco database JSON file path.
     * @param key   The ArUco marker set id use to set the dictionary, for example: DICT_6X6_250.
     */
    void set_dict(const std::string& path, const std::string& key) {
        std::ifstream fin(path);
        nlohmann::json db;
        fin >> db;
        set_dict(db, key);
    }
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
    void set_detector_ext(
        const std::int32_t&               pyramid_level
      , const std::int32_t&               border_bits               // bits
      , const std::int32_t&               fringe_bits               // bits
      , const double&                     a_bit_width               // pixels
      , const double&                     margin_size               // pixels 
      , const std::string&                frame_template_path
      , const std::string&                frame_mask_path
      , const std::int32_t&               nms_count
      , const double&                     nms_radius                // pixels
      , const std::int32_t&               cell_size                 // pixels
      , const std::vector<std::int32_t>&  ids                       // ids   
      , const std::vector<cv::Point>&     mk_index_map              // id to point
      , std::ostream&                     logger            = nucleona::stream::null_out
    ) {
        if(!dictionary_set_)
            throw std::runtime_error("dictionary not yet set");
        auto frame_template = cv::imread(frame_template_path, cv::IMREAD_GRAYSCALE);
        auto frame_mask     = cv::imread(frame_mask_path, cv::IMREAD_GRAYSCALE);
        detector_.reset(
            dictionary_, pyramid_level, 
            border_bits, fringe_bits,
            a_bit_width,
            margin_size, 
            frame_template, frame_mask,
            nms_count, nms_radius,
            cell_size,
            ids,
            logger
        );
        mk_index_map_ = mk_index_map;

        for(auto&& id : ids) {
            mk_pos_to_id_[mk_index_map_[id]] = id;
        }
    }
    /**
     * @brief Call operator, detect ArUco marker as regular matrix.
     * 
     * @tparam T                Input image value type.
     * @tparam Func             Function/Functor parameter pos_map's type, should be deduced.
     * 
     * @param src               The input image.
     * @param mk_invl_x         Marker interval along X direction, in pixel.
     * @param mk_invl_y         Marker interval along Y direction, in pixel.
     * @param mk_width          Marker width, in pixel. 
     * @param mk_height         Marker height, in pixel.
     * @param fov_mk_num_x      Marker numbers along the X direction.
     * @param fov_mk_num_y      Marker numbers along the Y direction.
     * @param pyramid_level     A number of downsampling levels for speeding up marker localization.
     *                          The value is an integer > 0. Experientially, setting the value of levels to 2 or 3 is good enough.
     * @param pos_map           The mapper function from FOV marker position to chip marker position.
     *                          The function symbol is cv::Point(int, int).
     * @return std::vector<MKRegion> 
     *                          The detected marker regions.
     */
    template<class T, class Func>
    std::vector<MKRegion> operator()(
        const cv::Mat_<T>&          src, 
        double                      mk_invl_x,
        double                      mk_invl_y,
        double                      mk_width,
        double                      mk_height,
        int                         fov_mk_num_x,
        int                         fov_mk_num_y,
        std::int32_t                pyramid_level,
        Func&&                      pos_map // FOV marker position to chip marker position
    ) const {
        auto raw_mk_rs = generate_raw_marker_regions(
            src, 
            mk_invl_x,
            mk_invl_y,
            mk_width,
            mk_height,
            fov_mk_num_x,
            fov_mk_num_y
        );
        auto view = src.clone();
        for(auto&& mk_r : raw_mk_rs) {
            auto tgt = src(mk_r);
            resize(tgt, pyramid_level);
            auto&& mk_chip_pos = pos_map(mk_r.x_i, mk_r.y_i);
            auto& aruco_mk_id = mk_pos_to_id_.at(mk_chip_pos);
            auto mk_img = detector_.aruco_img(aruco_mk_id);
            auto score = match_template(tgt, mk_img);
            auto max_points = make_fixed_capacity_set<cv::Point>(
                20, chipimgproc::utils::PosCompByScore(score)
            );
            for(int y = 0; y < score.rows; y ++ ) {
                for(int x = 0; x < score.cols; x ++ ) {
                    max_points.emplace(cv::Point(x, y));
                }
            }
            cv::Point max_loc;
            float max_score = 0;

            for(auto&& p : max_points) {
                max_loc.x += p.x;
                max_loc.y += p.y;
                max_score += score(p.y, p.x);
            }
            max_loc.x /= max_points.size();
            max_loc.y /= max_points.size();
            max_score /= max_points.size();

            auto size_r = 1 << pyramid_level; 
            mk_r.x      = max_loc.x * size_r + mk_r.x       ;
            mk_r.y      = max_loc.y * size_r + mk_r.y       ;
            mk_r.width  = mk_img.cols * size_r              ;
            mk_r.height = mk_img.rows * size_r              ;
            mk_r.score  = max_score;

            // std::cout << VDUMP(mk_r) << std::endl;
            // cv::rectangle(view, mk_r, 255);
            // cv::imwrite("aruco_det_debug.tiff", view);
        }
        return raw_mk_rs;
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
        std::vector<MKRegion> res;
        auto pts = detector_.detect_markers(src, out);
        res.reserve(pts.size());
        for(auto [i, p] : pts) {
            auto x = p.x - (marker_width  / 2);
            auto y = p.y - (marker_height / 2);
            auto& mk_idx = mk_index_map_.at(i);
            MKRegion mk_r;
            mk_r.x = x;
            mk_r.y = y;
            mk_r.width  = marker_width;
            mk_r.height = marker_height;
            mk_r.x_i = mk_idx.x;
            mk_r.y_i = mk_idx.y;
            mk_r.score = 0;
            res.push_back(mk_r);
        }
        return res;
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
    void set_dict_(aruco::Dictionary&& dict) {
        dictionary_ = std::move(dict);
        dictionary_set_ = true;
    }
    using PointToID = std::map<cv::Point, std::int32_t, PointLess>;

    aruco::Detector                         detector_               ;

    bool                                    dictionary_set_ {false} ;
    aruco::Dictionary                       dictionary_             ;

    std::vector<cv::Point>                  mk_index_map_           ;   // id to point
    PointToID                               mk_pos_to_id_           ;   // point to id
};

}