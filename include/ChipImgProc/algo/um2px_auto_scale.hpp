#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/txt_to_img.hpp>
#include <ChipImgProc/marker/detection/reg_mat_no_rot.hpp>
#include <Nucleona/proftool/timer.hpp>
#include <Nucleona/tuple.hpp>
namespace chipimgproc::algo {

struct Um2PxAutoScale {
    Um2PxAutoScale(
        const cv::Mat_<std::uint16_t>& image,
        const cv::Mat_<std::uint8_t>& marker,
        const cv::Mat_<std::uint8_t>& mk_mask,
        float cell_w_um,
        float cell_h_um,
        float border_um,
        int rows, 
        int cols,
        std::uint32_t invl_x_cl, 
        std::uint32_t invl_y_cl
    )
    : marker_       (marker)
    , mk_mask_      (mk_mask)
    , cell_w_um_    (cell_w_um) 
    , cell_h_um_    (cell_h_um)
    , border_um_    (border_um)
    , rows_         (rows)    
    , cols_         (cols)
    , invl_x_cl_    (invl_x_cl)
    , invl_y_cl_    (invl_y_cl)
    {
        image_ = norm_u8(image, 0.001, 0.001);
    }

    std::tuple<
        float, 
        cv::Mat, 
        marker::Layout
    > linear_steps(
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
        chipimgproc::marker::Layout max_layout;
        cv::Mat_<float> max_score_sum;

        auto cur_r = mid - (num * step);
        for(int i = 0; i < 2 * num; i ++ ) {
            auto layout = marker::make_single_pattern_reg_mat_layout(
                marker_, mk_mask_, cell_h_um_, cell_w_um_,
                border_um_, rows_, cols_, invl_x_cl_, invl_y_cl_,
                cur_r 
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
                max_layout = layout;
                max_score_sum = score_sum.clone();
            }
            cur_r += step;
        }
        log << "um2px_auto_scale result: " << max_um2px_r << std::endl;
        return std::make_tuple(
            max_um2px_r, 
            max_score_sum.clone(),
            max_layout
        );
    }

private:
    cv::Mat                        image_       ;
    const cv::Mat_<std::uint8_t>   marker_      ;
    const cv::Mat_<std::uint8_t>   mk_mask_     ;
    const float                    cell_w_um_   ;
    const float                    cell_h_um_   ;
    const float                    border_um_   ;
    const int                      rows_        ;
    const int                      cols_        ;
    std::uint32_t                  invl_x_cl_   ;
    std::uint32_t                  invl_y_cl_   ;
};

}