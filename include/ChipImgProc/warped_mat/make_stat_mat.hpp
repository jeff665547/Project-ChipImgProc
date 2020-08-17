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
        cv::Point2d origin, 
        int clw,    int clh,
        int clwd,   int clhd,
        int w,      int h,
        int swin_w, int swin_h,
        double      um2px_r,
        int clwn,   int clhn,
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
        int swin_w_px = std::round(swin_w * um2px_r);
        int swin_h_px = std::round(swin_h * um2px_r);
        int clw_px    = std::round(clw * um2px_r);
        int clh_px    = std::round(clh * um2px_r);
        auto [mean, sd] = make_large_cv_mat(mat, swin_w_px, swin_h_px);
        cv::Mat cv = sd / mean;

        auto lmask = make_large_mask(
            {
                static_cast<int>(std::round(origin.x)), 
                static_cast<int>(std::round(origin.y))
            },
            clw, clh, clwd, clhd,
            w, h, swin_w, swin_h, um2px_r, 
            clwn, clhn, warpmat, mat.size()
        );
        cv::Mat_<std::int32_t> mask_cell_label(lmask.size());
        cv::connectedComponents(lmask, mask_cell_label);
        {
            cv::Mat comp_img;
            mask_cell_label.convertTo(comp_img, CV_16U);
            cv::imwrite("components.png", comp_img);
            cv::imwrite("mask.png", lmask);
        }
        // lmask = lmask / 255;
        // ip_convert(lmask,           CV_32F);
        ip_convert(mask_cell_label, CV_32F);
        ip_convert(sd,              CV_32F);
        ip_convert(mean,            CV_32F);
        ip_convert(cv,              CV_32F);
        auto warped_agg_mat = make_basic(warpmat, 
            std::vector<cv::Mat>({
                mask_cell_label,
                lmask,
                cv,
                sd,
                mean
            }),
            origin, clwd, clhd, w, h
        );
        ObjMat<cv::Mat, std::uint32_t> warped_mask(clhn, clwn);
        stat::Mats<Float> stat_mats(clhn, clwn);
        for(int i = 0; i < clhn; i ++) {
            for(int j = 0; j < clwn; j ++) {
                int label = warped_agg_mat.at_cell(
                    i, j, cv::Size(1, 1)
                ).patch.at<float>(0, 0);
                auto mats = warped_agg_mat.at_cell_all(
                    i, j, cv::Size(clw_px, clh_px)
                );
                auto& sub_lab     = mats.at(0).patch;
                auto& sub_mask    = mats.at(1).patch;
                auto& sub_cv      = mats.at(2).patch;
                auto& sub_sd      = mats.at(3).patch;
                auto& sub_mean    = mats.at(4).patch;
                sub_lab = lab_to_mask(sub_lab, label);
                cv::Mat sum_mask = sub_mask & sub_lab;
                
                cv::Point min_cv_pos; // pixel
                double min_cv;
                cv::minMaxLoc(sub_cv, &min_cv, nullptr, &min_cv_pos, nullptr, sum_mask);
                stat_mats.mean  (i, j)  = sub_mean.template at<Float>(min_cv_pos);
                stat_mats.stddev(i, j)  = sub_sd.template at<Float>(min_cv_pos);
                stat_mats.cv    (i, j)  = min_cv;
                stat_mats.bg    (i, j)  = 0;
                stat_mats.num   (i, j)  = clh * clw;

                ip_convert(sum_mask, CV_32F, 1.0 / 255);
                warped_mask     (i, j)  = sum_mask;
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