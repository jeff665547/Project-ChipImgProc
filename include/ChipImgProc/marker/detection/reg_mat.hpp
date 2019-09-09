/**
 *  @file       ChipImgProc/marker/detection/reg_mat.hpp
 *  @author     Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 *  @brief      Detect markers in image and assume the marker layout is regular matrix distribution.
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/const.h>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <Nucleona/tuple.hpp>
#include <Nucleona/range.hpp>
#include <ChipImgProc/algo/fixed_capacity_set.hpp>
#include <ChipImgProc/utils/pos_comp_by_score.hpp>
namespace chipimgproc{ namespace marker{ namespace detection{
/**
 *  @brief      This class, named regular matrix (RegMat), is used to
 *              detect a rectangular array of markers, arranged in rows and columns.
 *              Given a raw image and corresponding marker layout descriptor,
 *              this class aims to return a collection of detected marker locations, sizes 
 *              and subcript indices.
 * 
 *  @details    Here is the example:
 *      @snippet ChipImgProc/marker/detection/reg_mat_test.cpp usage
 */
class RegMat {
    template<class T>
    auto generate_raw_marker_regions(
        const cv::Mat_<T>&      src, 
        const Layout&           mk_layout, 
        const MatUnit&          unit, 
        std::ostream&           out
    ) const {
        // marker interval between marker
        auto [mk_invl_x, mk_invl_y] = mk_layout.get_marker_invl(unit); // chip.json

        // marker w, h
        auto mk_width  = mk_layout.get_marker_width (unit) ; // chip.json
        auto mk_height = mk_layout.get_marker_height(unit) ; // chip.json

        // marker layout width and height
        auto mk_mat_w = mk_invl_x * (mk_layout.mk_map.cols - 1) + mk_width  ; // fov marker num
        auto mk_mat_h = mk_invl_y * (mk_layout.mk_map.rows - 1) + mk_height ;

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
    template<class T, class FUNC>
    auto template_matching(
        const cv::Mat_<T>&      src               , 
        const Layout&           mk_layout         , 
        const MatUnit&          unit              ,
        std::size_t             candi_mk_i        ,
        FUNC&&                  each_score_region ,
        std::ostream&           out               ,
        const ViewerCallback&   v_bin             ,
        const ViewerCallback&   v_search          ,
        const ViewerCallback&   v_marker   
    ) const {
        auto marker_regions = generate_raw_marker_regions(src, mk_layout, 
            unit, out);
        auto tgt = norm_u8(src); // TODO:
        info(out, tgt);
        if(v_bin) {
            v_bin(tgt);
        }
        cv::Mat_<std::uint8_t> view;
        if(v_search || v_marker) {
            view = tgt.clone();
        } 
        for(auto& mk_r : marker_regions) {
            auto& mk_des = mk_layout.get_marker_des(mk_r.y_i, mk_r.x_i);
            auto& candi_mks = mk_des.get_candi_mks(unit);
            auto& candi_mks_mask = mk_des.get_candi_mks_mask(unit);
            if(v_search) {
                cv::rectangle(view, mk_r, 128, 3);
            }
            cv::Mat sub_tgt = tgt(mk_r); 
            cv::Mat_<float> sub_score;
            auto& mk = candi_mks.at(candi_mk_i);
            auto& mask = candi_mks_mask.at(candi_mk_i);
            cv::Mat_<float> sub_candi_score(
                sub_tgt.rows - mk.rows + 1,
                sub_tgt.cols - mk.cols + 1
            );
            cv::matchTemplate(sub_tgt, mk, sub_candi_score, cv::TM_CCORR_NORMED, mask);
            // auto tmp = norm_u8(sub_candi_score, 0, 0);
            sub_score = sub_candi_score;
            auto mk_cols = mk_des.get_std_mk(unit).cols;
            auto mk_rows = mk_des.get_std_mk(unit).rows;
            each_score_region(
                sub_score, mk_r, mk_cols, mk_rows
            );
        }
        // filter_low_score_marker(marker_regions);
        if(v_marker) {
            for(auto& mk_r : marker_regions) {
                cv::rectangle(view, mk_r, 128, 1);
            }
        }
        if(v_search) {
            v_search(view);
        }
        if(v_marker) {
            v_marker(view);
        }
        return marker_regions;

    }
public:
    /**
     * @brief Call operator of RegMat type, 
     *        given marker layout and image, return marker regions.
     * 
     * @tparam T            (Deduced) Input matrix value type, for centrillion Summit image, 
     *                      the white channel usually use std::uint8_t. 
     *                      and probe channel use std::uint16_t.
     * @param src           Input image.
     * @param mk_layout     The marker layout of current process image, 
     *                      for different chip spec should have different chip marker layout. 
     * @param unit          Image unit level, can be MatUnit::PX (pixel level) or MatUnit::CELL (cell level).
     * @param cand_mk_i     Use the i-th candidate marker in marker layout to match the image. 
     *                      By default is 0, usually means the standard marker pattern.
     * @param out           The debug log message output, can be any STL ostream, by default is null stream
     * @param v_bin         The debug pre-process image output callback, the callback form is void(const cv::Mat&) type. 
     *                      Current implementation is 8bit normalization image.
     * @param v_search      The debug image output callback, the callback form is void(const cv::Mat&) type. 
     *                      Current implementation is show the marker searching space.
     * @param v_marker      The debug image output callback, the callback form is void(const cv::Mat&) type.
     *                      Current implementation is show the marker segmentation location
     * @return std::vector<MKRegion> 
     *                      A vector of marker regions
     */
    template<class T>
    std::vector<MKRegion> operator()(
        const cv::Mat_<T>&      src, 
        const Layout&           mk_layout, 
        const MatUnit&          unit,
        std::size_t             cand_mk_i  = 0,
        std::ostream&           out        = nucleona::stream::null_out,
        const ViewerCallback&   v_bin      = nullptr,
        const ViewerCallback&   v_search   = nullptr,
        const ViewerCallback&   v_marker   = nullptr
    ) const {
        return template_matching(
            src, mk_layout, unit, cand_mk_i, 
            [&out](auto&& sub_score, auto&& mk_r, auto&& mk_cols, auto&& mk_rows) {
                auto max_points = make_fixed_capacity_set<cv::Point>(
                    20, utils::PosCompByScore(sub_score)
                );
                cv::Point max_loc;
                float max_score = 0;
                for(int y = 0; y < sub_score.rows; y ++ ) {
                    for(int x = 0; x < sub_score.cols; x ++ ) {
                        max_points.emplace(cv::Point(x, y));
                    }
                }
                for(auto&& p : max_points) {
                    max_loc.x += p.x;
                    max_loc.y += p.y;
                    max_score += sub_score(p.y, p.x);
                }
                max_loc.x /= max_points.size();
                max_loc.y /= max_points.size();
                max_score /= max_points.size();

                max_loc.x  += mk_r.x                ;
                max_loc.y  += mk_r.y                ;
                mk_r.x      = max_loc.x             ;
                mk_r.y      = max_loc.y             ;
                mk_r.width  = mk_cols               ;
                mk_r.height = mk_rows               ;
                mk_r.info(out);
                mk_r.score  = max_score;
            }, out, v_bin, v_search, v_marker
        );
    }
};

}}}
