#pragma once
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc::marker::detection {
constexpr struct RegMatNoRot {
    std::vector<MKRegion> operator()(
        const cv::Mat_<std::uint16_t>&      src,
        const Layout&           mk_layout,
        const MatUnit&          unit,
        std::ostream&           out        = nucleona::stream::null_out,
        const ViewerCallback&   v_marker   = nullptr
    ) const {
        auto [mk_invl_x, mk_invl_y] = mk_layout.get_marker_invl(unit);
        auto mk_x_num = mk_layout.mk_map.cols;
        auto mk_y_num = mk_layout.mk_map.rows;
        auto mk_height = mk_layout.get_marker_height(unit);
        auto mk_width = mk_layout.get_marker_width(unit);
        auto mk_layout_width = (mk_x_num - 1) * (mk_width + mk_invl_x) + mk_width;
        auto mk_layout_height = (mk_y_num - 1) * (mk_height + mk_invl_y) + mk_height;
        auto scan_rect_width  = src.cols - mk_layout_width + mk_width;
        auto scan_rect_height = src.rows - mk_layout_height + mk_height;

        cv::Mat src_8u = norm_u8(src, 0.01, 0.01);
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
                auto& mk_mask = mk_des.get_candi_mks_mask_px();
                cv::matchTemplate(scan_target_mat, mk_pat, score, CV_TM_CCORR_NORMED, mk_mask);
            } else if (unit == MatUnit::CELL) {
                cv::matchTemplate(scan_target_mat, mk_pat, score, CV_TM_CCORR_NORMED);
            }
            score_sum += score;
        }
        // TODO: max score point search
    }
} reg_mat_no_rot;
}