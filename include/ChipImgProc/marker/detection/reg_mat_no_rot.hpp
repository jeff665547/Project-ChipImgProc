#pragma once
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/marker/detection/pos_comp_by_score.hpp>
#include <ChipImgProc/algo/fixed_capacity_set.hpp>
#include <ChipImgProc/marker/detection/pos_comp_by_score.hpp>
namespace chipimgproc::marker::detection {
constexpr struct RegMatNoRot {
    std::vector<MKRegion> operator()(
        const cv::Mat_<std::uint16_t>&      src,
        Layout&                 mk_layout,
        const MatUnit&          unit,
        std::ostream&           out        = nucleona::stream::null_out,
        const ViewerCallback&   v_marker   = nullptr
    ) const {
        auto [mk_invl_x, mk_invl_y] = mk_layout.get_marker_invl(unit);
        auto mk_x_num = mk_layout.mk_map.cols;
        auto mk_y_num = mk_layout.mk_map.rows;
        auto mk_height = mk_layout.get_marker_height(unit);
        auto mk_width = mk_layout.get_marker_width(unit);
        auto mk_layout_width = (mk_x_num - 1) *  mk_invl_x + mk_width;
        auto mk_layout_height = (mk_y_num - 1) * mk_invl_y + mk_height;
        auto scan_rect_width  = src.cols - mk_layout_width + mk_width;
        auto scan_rect_height = src.rows - mk_layout_height + mk_height;

        cv::Mat src_8u = norm_u8(src, 0.001, 0.001);
        cv::Mat_<float> score_sum(
            scan_rect_height - mk_height + 1,
            scan_rect_width - mk_width + 1
        );
        for(auto&& mk_des : mk_layout.mks) {
            auto scan_start_point = mk_des.get_pos(unit);
            cv::Rect scan_region(
                scan_start_point.x, 
                scan_start_point.y,
                scan_rect_width,
                scan_rect_height
            );
            cv::Mat scan_target_mat = src_8u(scan_region);
            auto& mk_pat = mk_des.get_candi_mks(unit).at(0);
            cv::Mat_<float> score(
                scan_target_mat.rows - mk_pat.rows + 1,
                scan_target_mat.cols - mk_pat.cols + 1
            );
            if( unit == MatUnit::PX) {
                auto& mk_mask = mk_des.get_candi_mks_mask_px().at(0);
                cv::matchTemplate(scan_target_mat, mk_pat, score, CV_TM_CCORR_NORMED, mk_mask);
            } else if (unit == MatUnit::CELL) {
                cv::matchTemplate(scan_target_mat, mk_pat, score, CV_TM_CCORR_NORMED);
            }
            score_sum += score;
        }
        auto max_points = make_fixed_capacity_set<cv::Point>(
            20, PosCompByScore(score_sum)
        );
        cv::Point max_loc;
        float max_score = 0;
        for(int y = 0; y < score_sum.rows; y ++ ) {
            for(int x = 0; x < score_sum.cols; x ++ ) {
                auto& score = score_sum(y, x);
                max_points.emplace(cv::Point(x, y));
            }
        }
        for(auto&& p : max_points) {
            max_loc.x += p.x;
            max_loc.y += p.y;
            max_score += score_sum(p.y, p.x);
        }
        max_loc.x /= max_points.size();
        max_loc.y /= max_points.size();

        std::vector<MKRegion> mk_regs;
        for( int i = 0; i < mk_layout.mk_map.rows; i ++ ) {
            for( int j = 0; j < mk_layout.mk_map.cols; j ++ ) {
                auto& mk = mk_layout.get_marker_des(i, j);
                // mk.get_pos(unit).x += max_loc.x;
                // mk.get_pos(unit).y += max_loc.y;
                MKRegion mk_reg;
                mk_reg.x = mk.get_pos(unit).x + max_loc.x;
                mk_reg.y = mk.get_pos(unit).y + max_loc.y;
                mk_reg.width = mk_layout.get_marker_width(unit);
                mk_reg.height = mk_layout.get_marker_height(unit);
                mk_reg.x_i = j;
                mk_reg.y_i = i;
                mk_regs.push_back(mk_reg);
            }
        }

        if(v_marker) {
            auto view = viewable(src);
            for(auto& mk_r : mk_regs) {
                cv::rectangle(view, mk_r, 32768, 1);
            }
            v_marker(view);
        }
        return mk_regs;
    }
} reg_mat_no_rot;
}