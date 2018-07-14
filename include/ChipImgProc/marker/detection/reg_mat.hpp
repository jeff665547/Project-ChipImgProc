#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <Nucleona/tuple.hpp>
#include <ChipImgProc/const.h>
#include <ChipImgProc/marker/detection/mk_region.hpp>
namespace chipimgproc{ namespace marker{ namespace detection{

struct RegMat {
    auto get_mk_invl( const Layout& mk_layout, const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::PX:
                return nucleona::make_tuple(
                    mk_layout.mk_invl_x_px,
                    mk_layout.mk_invl_y_px
                );
            case MatUnit::CELL:
                return nucleona::make_tuple(
                    mk_layout.mk_invl_x_cl,
                    mk_layout.mk_invl_y_cl
                );
            default:
                throw std::runtime_error(
                    "get_mk_invl, unsupported unit: " + unit.to_string()
                );
        }
    }
    template<class T>
    std::vector<MKRegion> operator()(
        const cv::Mat_<T>&      src, 
        const Layout&           mk_layout, 
        const MatUnit&          unit,
        std::ostream&           out        = nucleona::stream::null_out,
        const ViewerCallback&   v_bin      = nullptr,
        const ViewerCallback&   v_search   = nullptr,
        const ViewerCallback&   v_marker   = nullptr
    ) const {
        // marker interval between marker
        auto [mk_invl_x, mk_invl_y] = get_mk_invl(mk_layout, unit);

        // marker w, h
        auto mk_width  = mk_layout.get_marker_width (unit) ;
        auto mk_height = mk_layout.get_marker_height(unit) ;

        // marker layout width and height
        auto mk_mat_w = mk_invl_x * (mk_layout.mk_map.cols - 1) + mk_width  ;
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
                marker_region.info(out);
                marker_regions.push_back(marker_region);
                x_last_i = x_i;
            }
            y_last_i = y_i;
        }
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
            if(v_search) {
                cv::rectangle(view, mk_r, 128, 3);
            }
            cv::Mat sub_tgt = tgt(mk_r); 
            cv::Mat_<float> sub_score;
            for(auto& mk : candi_mks) {
                cv::Mat_<float> sub_candi_score(
                    sub_tgt.rows - mk.rows + 1,
                    sub_tgt.cols - mk.cols + 1
                );
                cv::matchTemplate(sub_tgt, mk, sub_candi_score, CV_TM_CCORR_NORMED);
                if(sub_score.empty()) 
                    sub_score = sub_candi_score;
                else 
                    sub_score += sub_candi_score;
            }
            double min, max;
            cv::Point min_loc, max_loc;
            cv::minMaxLoc(sub_score, &min, &max, &min_loc, &max_loc);
            max_loc.x  += mk_r.x                ;
            max_loc.y  += mk_r.y                ;
            mk_r.x      = max_loc.x             ;
            mk_r.y      = max_loc.y             ;
            mk_r.width  = candi_mks.at(0).cols  ;
            mk_r.height = candi_mks.at(0).rows  ;
            mk_r.info(out);
            if(v_marker) {
                cv::rectangle(view, mk_r, 128, 3);
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
};

}}}