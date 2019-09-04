/**
 * @file reg_mat.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::gridding::RegMat
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/gridding/result.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/logger.hpp>
#include "utils.hpp"
namespace chipimgproc{ namespace gridding{
/**
 * @brief Grid the input image with given marker layout(chipimgproc::marker::Layout) 
 *        and marker regions(chipimgproc::marker::detection::MKRegion)
 * 
 */
class RegMat {
    using MKRegion = marker::detection::MKRegion;
    using MKLayout = marker::Layout;
    template<class FUNC>
    auto grid_anchors_group( 
        MKRegion::Group<cv::Point>& grouped_ps, 
        FUNC&& coor_func,
        std::uint32_t mk_invl_cl,
        int marker_edge_cl
    ) const {
        std::vector<std::vector<float>> res;
        for( auto&& p : grouped_ps ) {
            res.emplace_back();
            auto& grid_anchors = res.back();
            std::sort(p.second.begin(), p.second.end(), [&coor_func](
                const auto& a, const auto& b
            ){
                return coor_func(a) < coor_func(b);
            });
            float cl_e_px = 0;
            for( int i = 1; i < p.second.size(); i ++ ) {
                auto init_achr = coor_func(p.second.at(i-1));
                cl_e_px = (coor_func(p.second.at(i)) - init_achr) / (float)mk_invl_cl;
                for(std::uint32_t j = 0; j < mk_invl_cl; j ++ ) {
                    grid_anchors.push_back(init_achr + (j * cl_e_px));
                }
            }
            auto last_marker_start = coor_func(p.second.back());
            for(int i = 0; i <= marker_edge_cl; i ++ ) {
                grid_anchors.push_back(last_marker_start + (cl_e_px * i));
            }
        }
        return res;
    }

    auto consensus( const std::vector<std::vector<float>>& anchors_group ) const {
        auto size = anchors_group.front().size();
        for( auto&& as : anchors_group ) {
            if( as.size() != size ) {
                throw std::runtime_error("assert fail, anchor number not match");
            }
        }

        std::vector<std::uint32_t> res;
        for(decltype(size) i = 0; i < size; i ++ ) {
            float sum = 0;
            for( auto&& as : anchors_group ) {
                sum += as.at(i);
            }
            res.push_back(std::round(sum / anchors_group.size()));
        }
        return res;
    }
    template<class T>
    auto shift_0( std::vector<T>& anchors) const {
        auto org = anchors.front();
        for( auto& v : anchors ) {
            v -= org;
        }
        return org;
    }
public:
    /**
     * @brief Call operator, @copybrief chipimgproc::gridding::RegMat
     * 
     * @param in_src        Input image data
     * @param mk_layout     Image marker layout, provide logical marker position.
     * @param mk_regs       Image marker regions, 
     *                      provide real marker position detected from marker detection algorithm.
     * @param msg           Deprecated, debug output message.
     * @param v_result      Debug grid line image output.
     * @return Result       A data structure contains:  
     *                      grid table row and column number, each tiles and grid line position.
     */
    Result operator()(
          const cv::Mat&                in_src
        , const MKLayout&               mk_layout
        , std::vector<MKRegion>&        mk_regs
        , std::ostream&                 msg         = nucleona::stream::null_out
        , const std::function<
            void(const cv::Mat&)
          >&                            v_result    = nullptr
    ) const {
        Result result;
        auto [mk_invl_x, mk_invl_y] = mk_layout.get_marker_invl(MatUnit::CELL);
        int fov_w_cl = ( mk_invl_x * (mk_layout.mk_map.cols - 1) ) + mk_layout.get_marker_width_cl();
        int fov_h_cl = ( mk_invl_y * (mk_layout.mk_map.rows - 1) ) + mk_layout.get_marker_height_cl();
        // find left top and right botton  mk region
        MKRegion* left_top = &(mk_regs.front());
        MKRegion* right_bottom = &(mk_regs.front());
        for(auto&& mk_r : mk_regs) {
            if(
                mk_r.x_i <= left_top->x_i && 
                mk_r.y_i <= left_top->y_i
            ) {
                left_top = &mk_r;
            }
            if(
                mk_r.x_i >= right_bottom->x_i && 
                mk_r.y_i >= right_bottom->y_i
            ) {
                right_bottom = &mk_r;
            }
        }
        std::vector<double> x_grid_anchor;
        std::vector<double> y_grid_anchor; 
        auto fov_h_px = right_bottom->y + right_bottom->height - left_top->y;
        auto fov_w_px = right_bottom->x + right_bottom->width - left_top->x;

        chipimgproc::log.trace("FOV height: {}px", fov_h_px);
        chipimgproc::log.trace("FOV width : {}px", fov_w_px);

        double cl_h_px = fov_h_px / (double)fov_h_cl;
        double cl_w_px = fov_w_px / (double)fov_w_cl;
        for(int i = 0; i < fov_h_cl + 1; i ++ ) {
            y_grid_anchor.push_back(left_top->y + (i * cl_h_px));
        }
        for(int j = 0; j < fov_w_cl + 1; j ++ ) {
            x_grid_anchor.push_back(left_top->x + (j * cl_w_px));
        }

        result.feature_rows = y_grid_anchor.size() - 1;
        result.feature_cols = x_grid_anchor.size() - 1;
        result.tiles        = gridline_to_tiles(x_grid_anchor, y_grid_anchor);
        result.gl_x         = x_grid_anchor;
        result.gl_y         = y_grid_anchor;

        chipimgproc::log.trace("FOV grid rows: {}", result.feature_rows);
        chipimgproc::log.trace("FOV grid cols: {}", result.feature_cols);

        if(v_result) {
            cv::Mat_<std::uint16_t> debug_img = viewable(in_src);
            auto color = 65536/2;
            for (auto tile: result.tiles)
            {
                tile.width  += 1;
                tile.height += 1;
                cv::rectangle(debug_img, tile, color);
            }
            v_result(debug_img);

        }
        return result;
    }
};

}}