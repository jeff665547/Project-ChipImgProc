#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <Nucleona/tuple.hpp>
#include "make_mask.hpp"
#include "basic.hpp"
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/obj_mat.hpp>

namespace chipimgproc::warped_mat {

template<class Float>
struct MakeStatMat {
    auto operator()(
        cv::Mat     mat,
        cv::Point   origin, 
        int clw,    int clh,
        int clwd,   int clhd,
        int clwn,   int clhn,
        int w,      int h,
        int swin_w, int swin_h,
        cv::Mat     warpmat
    ) const {
        /*
            1.1 generate large cv matrix
            1.2 generate mask

            2.1 warp mask

            2.2 mask convolution

            2.3 mask thresholding

                repeat 2.1 *4

            2.4 combine mask

            3. labeling mask object

            4. warp point subpixel cell roi to labeled mask & binarize
            5. warp point subpixel cell roi to large cv matrix & minmaxloc with mask
            6. for each cell do 7,8

            7. make warped matrix
        */
        auto [mean, sd] = make_large_cv_mat(mat, swin_w, swin_h);
        auto cv = sd / mean;

        auto lmask   = make_large_mask(origin, clw, clh, clwd, clhd,
            clwn, clhn, w, h, swin_w, swin_h, warpmat
        );
        cv::Mat_<std::int32_t> mask_cell_label(lmask.size());
        cv::connectedComponents(lmask, mask_cell_label);

        auto warped_agg_mat = make_basic(warpmat, 
            std::vector<cv::Mat>({
                mask_cell_label,
                lmask,
                cv,
                sd,
                mean
            }),
            origin, clwd, clhd
        );
        ObjMat<cv::Mat, std::uint32_t> warped_mask(clhn, clwn);
        stat::Mats stat_mats(clhn, clwn);
        for(int i = 0; i < clhn; i ++) {
            for(int j = 0; j < clwn; j ++) {
                std::int32_t label = warped_agg_mat.at_cell(
                    i, j, 0, cv::Size(1, 1)
                ).at<std::int32_t>(0, 0);
                auto mats = warped_agg_mat.at_cell(
                    i, j, cv::Size(clw, clh)
                );
                auto& sub_lab     = mats.at(0);
                auto& sub_mask    = mats.at(1);
                auto& sub_cv      = mats.at(2);
                auto& sub_sd      = mats.at(3);
                auto& sub_mean    = mats.at(4);
                sub_lab = lab_to_mask(sub_lab, label);
                cv::Mat sum_mask = sub_mask & sub_lab;
                
                cv::Point min_cv_pos;
                double min_cv;
                cv::minMaxLoc(sub_cv, &min_cv, nullptr, &min_cv_pos, nullptr, sum_mask);
                stat_mats.mean(i, j)    = sub_mean.at<Float>(min_cv_pos);
                stat_mats.stddev(i, j)  = sub_sd.at<Float>(min_cv_pos);
                stat_mats.cv(i, j)      = min_cv;
                stat_mats.bg(i, j)      = 0;
                stat_mats.num(i, j)     = clh * clw;
                warped_mask(i, j) = sum_mask;
            }
        }
        return nucleona::make_tuple(std::move(stat_mats), std::move(warped_mask));
    }
private:
    cv::Mat lab_to_mask(cv::Mat lab, std::int32_t i) const {
        return lab == i;
    }
    auto make_large_cv_mat(
        cv::Mat mat, int conv_w, int conv_h
    ) const {
        auto x_mean = mean(mat, conv_w, conv_h);
        auto x_2 = mat.mul(mat);
        auto x_2_mean = mean(x_2, conv_w, conv_h);
        cv::Mat var = x_2_mean - x_mean;
        cv::Mat sd(var.size(), var.type());
        cv::sqrt(var, sd);
        return nucleona::make_tuple(
            std::move(x_mean),
            std::move(sd)
        );
    }
    auto mean(
        cv::Mat mat, int conv_w, int conv_h
    ) const {
        cv::Mat kern(conv_h, conv_w, type_to_depth<Float>());
        kern.setTo(1.0 / (conv_w * conv_h));
        return filter2D(mat, kern, type_to_depth<Float>());
    } 
    MakeMask make_large_mask;
};

}