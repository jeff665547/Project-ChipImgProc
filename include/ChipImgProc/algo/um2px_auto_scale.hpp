#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/txt_to_img.hpp>
#include <ChipImgProc/marker/detection/reg_mat_no_rot.hpp>
#include <Nucleona/proftool/timer.hpp>
#include <Nucleona/tuple.hpp>
namespace chipimgproc::algo {

// assume the marker is single pattern regular matrix layout
struct Um2PxAutoScale {
    Um2PxAutoScale(
        const cv::Mat_<std::uint16_t>& image,
        float cell_w_um,
        float cell_h_um,
        float border_um
    )
    : cell_w_um_    (cell_w_um) 
    , cell_h_um_    (cell_h_um)
    , border_um_    (border_um)
    {
        image_ = norm_u8(image);
    }

    std::tuple<
        float, 
        cv::Mat
    > linear_steps(
        marker::Layout& mk_layout,
        float mid, float step, int num,
        const std::vector<cv::Point>& ignore_mk_regs = {},
        std::ostream& log = nucleona::stream::null_out
    ) const {
        auto holder = nucleona::proftool::make_timer([&log](auto&& du){
            auto milli = std::chrono::duration_cast<std::chrono::milliseconds>(du);
            log << "Um2PxAutoScale::linear_steps time: " << milli.count() << "ms" << std::endl;
        });
        double max_score = 0;
        float max_um2px_r = 0;
        cv::Mat_<float> max_score_sum;

        auto cur_r = mid - (num * step);
        for(int i = 0; i < 2 * num; i ++ ) {
            auto [mks_cl, masks_cl] = mk_layout.get_single_pat_marker_des(
                MatUnit::CELL
            );
            marker::Layout layout = mk_layout;
            chipimgproc::marker::make_single_pattern_reg_mat_layout(
                layout, cell_h_um_, cell_w_um_,
                border_um_, cur_r 
            );
            auto score_sum = marker::detection::reg_mat_no_rot.score_mat(
                image_, layout, MatUnit::PX, ignore_mk_regs, log
            );
            // {
            //     auto score_view = norm_u8(score_sum, 0, 0);
            //     cv::imwrite("score_" + std::to_string(cur_r) + ".tiff", score_view);
            // }
            double cur_max;
            cv::Point max_loc;
            cv::minMaxLoc(score_sum, 0, &cur_max, 0, &max_loc);
            if( cur_max > max_score ) {
                max_score = cur_max;
                max_um2px_r = cur_r;
                mk_layout = layout;
                max_score_sum = score_sum.clone();
            }
            cur_r += step;
        }
        log << "um2px_auto_scale result: " << max_um2px_r << std::endl;
        return std::make_tuple(
            max_um2px_r, 
            max_score_sum.clone()
        );
    }

private:
    cv::Mat                        image_       ;
    const float                    cell_w_um_   ;
    const float                    cell_h_um_   ;
    const float                    border_um_   ;
};

}