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
        std::ostream&           out        = nucleona::stream::null_out
    ) const {
        // marker interval between marker
        auto [mk_invl_x, mk_invl_y] = get_mk_invl(mk_layout, unit);

        // marker layout width and height
        auto mk_mat_w = mk_invl_x * (mk_layout.mk_map.cols - 1);
        auto mk_mat_h = mk_invl_y * (mk_layout.mk_map.rows - 1);

        // marker layout origin
        auto x_org = ( src.cols / 2 ) - ( mk_mat_w / 2 );
        auto y_org = ( src.rows / 2 ) - ( mk_mat_h / 2 );

        // cut points
        std::vector<std::uint32_t> cut_points_x;
        std::vector<std::uint32_t> cut_points_y;
        std::vector<MKRegion> marker_regions;
        std::uint32_t last_x = 0;
        for( std::uint32_t x = x_org; x < (x_org + mk_mat_w); x += mk_invl_x ) {
            cut_points_x.push_back((last_x + x) / 2);
            last_x = x;
        }
        std::uint32_t last_y = 0;
        for( std::uint32_t y = y_org; y < (y_org + mk_mat_h); y += mk_invl_y ) {
            cut_points_y.push_back((last_y + y) / 2);
            last_y = y;
        }
        std::size_t y_i_last = 0;
        for(std::size_t y_i = 1; y_i < cut_points_y.size(); y_i ++ ) {
            std::size_t x_i_last = 0;
            for(std::size_t x_i = 1; x_i < cut_points_x.size(); x_i ++ ) {
                MKRegion marker_region;
                marker_region.x      = cut_points_x.at(x_i);
                marker_region.y      = cut_points_y.at(y_i);
                marker_region.width  = cut_points_x.at(x_i) - cut_points_x.at(x_i_last);
                marker_region.height = cut_points_y.at(y_i) - cut_points_y.at(y_i_last);
                marker_region.x_i    = x_i_last;
                marker_region.y_i    = y_i_last;
                marker_regions.push_back(marker_region);
                x_i_last = x_i;
            }
            y_i_last = y_i;
        }
        auto tgt = binarize(src); // TODO:
        for(auto& mk_r : marker_regions) {
            auto& mk_des = mk_layout.get_marker_des(mk_r.y_i, mk_r.x_i);
            auto& candi_mks = mk_des.get_candi_mks(unit);
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
        }
        return marker_regions;
    }
};

}}}