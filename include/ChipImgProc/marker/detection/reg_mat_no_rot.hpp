/**
 * @file reg_mat_no_rot.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief  Detect markers in image and assume the marker layout is regular matrix distribution and ignore rotation.
 * 
 */

#pragma once
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/utils/pos_comp_by_score.hpp>
#include <ChipImgProc/algo/fixed_capacity_set.hpp>
namespace chipimgproc::marker::detection {

/**
 * @brief type of immutable global variable reg_mat_no_rot.
 * 
 */
constexpr struct RegMatNoRot {
    /**
     * @brief Given image and marker layout, output score matrix
     * 
     * @param src_u8         Image normalize to unsigned 8 bits matrix
     * @param mk_layout      The marker layout of current process image, 
     *                       for different chip spec should have different chip marker layout. 
     * @param unit           Image unit level, can be MatUnit::PX (pixel level) or MatUnit::CELL (cell level).
     * @param ignore_mk_regs Ignored marker, the marker position id in this list 
     *                       will not take to compute the score matrix.
     * @param out            Log message output(deprecated).
     * @return cv::Mat_<float> 
     *                       Score matrix use to inference the marker regions.
     */
    cv::Mat_<float> score_mat(
        cv::Mat                             src_u8,
        Layout&                             mk_layout,
        const MatUnit&                      unit,
        const std::vector<cv::Point>&       ignore_mk_regs = {},
        std::ostream&                       out            = nucleona::stream::null_out
    ) const {
        auto [mk_invl_x, mk_invl_y] = mk_layout.get_marker_invl(unit);
        auto mk_x_num = mk_layout.mk_map.cols;
        auto mk_y_num = mk_layout.mk_map.rows;
        auto mk_height = mk_layout.get_marker_height(unit);
        auto mk_width = mk_layout.get_marker_width(unit);
        auto mk_layout_width = (mk_x_num - 1) *  mk_invl_x + mk_width;
        auto mk_layout_height = (mk_y_num - 1) * mk_invl_y + mk_height;
        auto scan_rect_width  = src_u8.cols - mk_layout_width + mk_width;
        auto scan_rect_height = src_u8.rows - mk_layout_height + mk_height;

        cv::Mat_<float> score_sum(
            scan_rect_height - mk_height + 1,
            scan_rect_width - mk_width + 1
        );
        score_sum = 0;
        for(int i = 0; i < mk_layout.mk_map.rows; i ++ ) {
            for(int j = 0; j < mk_layout.mk_map.cols; j ++ ) {
                auto& mk_des = mk_layout.get_marker_des(i, j);
                if(std::find(
                    ignore_mk_regs.begin(), ignore_mk_regs.end(), 
                    cv::Point(j, i)
                ) != ignore_mk_regs.end()) continue; // hit the ignore list
                auto scan_start_point = mk_des.get_pos(unit);
                cv::Rect scan_region(
                    scan_start_point.x, 
                    scan_start_point.y,
                    scan_rect_width,
                    scan_rect_height
                );
                cv::Mat scan_target_mat = src_u8(scan_region);
                auto& mk_pat = mk_des.get_best_mk(unit);
                cv::Mat_<float> score(
                    scan_target_mat.rows - mk_pat.rows + 1,
                    scan_target_mat.cols - mk_pat.cols + 1
                );
                if( unit == MatUnit::PX) {
                    auto& mk_mask = mk_des.get_best_mk_mask(unit);
                    // chipimgproc::algo::scaled_match_template.max(
                    //     scan_target_mat, mk_pat, score, 
                    //     CV_TM_CCORR_NORMED, 2, mk_mask
                    // );
                    cv::matchTemplate(scan_target_mat, mk_pat, score, cv::TM_CCORR_NORMED, mk_mask);
                } else if (unit == MatUnit::CELL) {
                    cv::matchTemplate(scan_target_mat, mk_pat, score, cv::TM_CCORR_NORMED);
                }
                score_sum += score;
            }
        }
        return score_sum;
    }
    /**
     * @brief Inference the marker regions
     * 
     * @param score_matrix  The score matrix.
     * @param mk_layout     The marker layout of current process image, 
     *                      for different chip spec should have different chip marker layout. 
     * @param unit          Image unit level, can be MatUnit::PX (pixel level) or MatUnit::CELL (cell level).
     * @param out           Log message output(deprecated).
     * @return std::vector<MKRegion> 
     *                      A vector of marker regions
     */
    std::vector<MKRegion> infer_marker_regions(
        const cv::Mat_<float>&  score_matrix,
        Layout&                 mk_layout,
        const MatUnit&          unit,
        std::ostream&           out        = nucleona::stream::null_out
    ) const {
        auto& score_sum = score_matrix;
        cv::Point max_loc;
        cv::minMaxLoc(score_sum, nullptr, nullptr, nullptr, &max_loc);

        // auto max_points = make_fixed_capacity_set<cv::Point>(
        //     20, chipimgproc::utils::PosCompByScore(score_sum)
        // );
        // cv::Point max_loc;
        // float max_score = 0;
        // for(int y = 0; y < score_sum.rows; y ++ ) {
        //     for(int x = 0; x < score_sum.cols; x ++ ) {
        //         auto& score = score_sum(y, x);
        //         max_points.emplace(cv::Point(x, y));
        //     }
        // }
        // for(auto&& p : max_points) {
        //     max_loc.x += p.x;
        //     max_loc.y += p.y;
        //     max_score += score_sum(p.y, p.x);
        // }
        // max_loc.x /= max_points.size();
        // max_loc.y /= max_points.size();


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

        return mk_regs;
    }
    /**
     * @brief Call operator of RegMatNoRot, 
     *        given marker layout and image, return the marker regions.
     * 
     * @param src              Image input, must be uint16 value type matrix. 
     * @param mk_layout        The marker layout of current process image, 
     *                         for different chip spec should have different chip marker layout. 
     * @param unit             Image unit level, can be MatUnit::PX (pixel level) or MatUnit::CELL (cell level).
     * @param ignore_mk_regs   Ignored marker, the marker position id in this list 
     *                         will not take to compute the score matrix.
     * @param out              Log message output(deprecated).
     * @param v_marker         The debug image output callback, the callback form is void(const cv::Mat&) type.
     *                         Current implementation is show the marker segmentation location
     * @return std::vector<MKRegion> 
     *                         A vector of marker regions
     */
    std::vector<MKRegion> operator()(
        const cv::Mat_<std::uint16_t>&      src,
        Layout&                             mk_layout,
        const MatUnit&                      unit,
        const std::vector<cv::Point>&       ignore_mk_regs  = {},
        std::ostream&                       out             = nucleona::stream::null_out,
        const ViewerCallback&               v_marker        = nullptr
    ) const {
        cv::Mat src_u8 = norm_u8(src);
        auto score_sum = score_mat(
            src_u8, mk_layout, unit, ignore_mk_regs, out
        );
        auto mk_regs = infer_marker_regions(
            score_sum, mk_layout, unit, out 
        );
        if(v_marker) {
            auto view = viewable(src);
            for(auto& mk_r : mk_regs) {
                cv::rectangle(view, mk_r, 32768, 1);
            }
            v_marker(view);
        }
        return mk_regs;
    }
} 
/**
 * @brief Global functor with RegMatNoRot type.
 * 
 */
reg_mat_no_rot;
}