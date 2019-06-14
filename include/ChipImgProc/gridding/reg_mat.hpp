#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/gridding/result.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/marker/layout.hpp>
namespace chipimgproc{ namespace gridding{

struct RegMat {
    using MKRegion = marker::detection::MKRegion;
    using MKLayout = marker::Layout;
    // using FUNC = std::function<int(cv::Point)>;
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
    Result operator()(
          cv::Mat&                      in_src
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
        std::vector<std::uint32_t> x_grid_anchor;
        std::vector<std::uint32_t> y_grid_anchor; 
        auto fov_h_px = right_bottom->y + right_bottom->height - left_top->y;
        auto fov_w_px = right_bottom->x + right_bottom->width - left_top->x;
        msg << VDUMP(fov_h_px) << std::endl;
        msg << VDUMP(fov_w_px) << std::endl;
        float cl_h_px = fov_h_px / (float)fov_h_cl;
        float cl_w_px = fov_w_px / (float)fov_w_cl;
        for(int i = 0; i < fov_h_cl + 1; i ++ ) {
            y_grid_anchor.push_back((std::uint32_t)std::round(left_top->y + (i * cl_h_px)));
        }
        for(int j = 0; j < fov_w_cl + 1; j ++ ) {
            x_grid_anchor.push_back((std::uint32_t)std::round(left_top->x + (j * cl_w_px)));
        }
        cv::Rect roi(
            x_grid_anchor.front(), y_grid_anchor.front(),
            x_grid_anchor.back() - x_grid_anchor.front(),
            y_grid_anchor.back() - y_grid_anchor.front()
        );
        auto tmp = in_src(roi);
        in_src.release();
        in_src = tmp;

        auto x_offset = shift_0(x_grid_anchor);
        auto y_offset = shift_0(y_grid_anchor);
        for ( auto& mk : mk_regs ) {
            mk.x -= x_offset;
            mk.y -= y_offset;
            msg << mk << std::endl;
        }


        std::vector<cv::Rect> tiles;
        for(std::size_t i = 1; i < y_grid_anchor.size(); i ++ ) {
            for (std::size_t j = 1; j < x_grid_anchor.size(); j ++ ) {
                auto w = x_grid_anchor.at(j) - x_grid_anchor.at(j-1);
                auto h = y_grid_anchor.at(i) - y_grid_anchor.at(i-1);
                tiles.push_back(
                    cv::Rect(
                        x_grid_anchor.at(j-1),
                        y_grid_anchor.at(i-1),
                        w, h
                    )
                );
            }
        }
        result.feature_rows = y_grid_anchor.size() - 1;
        result.feature_cols = x_grid_anchor.size() - 1;
        result.tiles        = tiles;
        result.gl_x         = x_grid_anchor;
        result.gl_y         = y_grid_anchor;

        msg << "feature_rows: " << result.feature_rows << std::endl;
        msg << "feature_cols: " << result.feature_cols << std::endl;

        if(v_result) {
            cv::Mat_<std::uint16_t> debug_img = viewable(in_src);
            auto color = 65536/2;
            for (auto tile: tiles)
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