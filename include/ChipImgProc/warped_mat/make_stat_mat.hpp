#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <Nucleona/tuple.hpp>
#include "make_mask.hpp"
#include "basic.hpp"
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/obj_mat.hpp>
#include <opencv2/gapi/core.hpp>
#include <opencv2/gapi/imgproc.hpp>
#include <opencv2/gapi/gpu/imgproc.hpp>

namespace chipimgproc::warped_mat {

template<class Float>
struct MakeStatMat {
    auto operator()(
        cv::Mat     mat,
        cv::Point2d     origin, 
        int clw,        int clh,
        int clwd,       int clhd,
        int w,          int h,
        int swin_w,     int swin_h,
        double          um2px_r,
        int clwn,       int clhn,
        cv::Mat         warpmat,
        ViewerCallback  v_margin = {},
        ViewerCallback  v_comp   = {},
        ViewerCallback  v_mask   = {}
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
        // auto tmp_timer(std::chrono::steady_clock::now());
        // std::chrono::duration<double, std::milli> d;
        auto [mean, sd] = make_large_cv_mat(mat, swin_w_px, swin_h_px);
        cv::Mat cv = sd / mean;
        // d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "make_large_cv_mat: " << d.count() << " ms\n";

        // tmp_timer = std::chrono::steady_clock::now();
        auto lmask = make_large_mask(
            {
                static_cast<int>(std::round(origin.x)), 
                static_cast<int>(std::round(origin.y))
            },
            clw, clh, clwd, clhd,
            w, h, swin_w, swin_h, um2px_r, 
            clwn, clhn, warpmat, mat.size()
        );
        // cv::Mat test_img;
        // lmask.convertTo(test_img, CV_16U);
        // cv::imwrite("large_mask.tiff", test_img);
        // d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "make_large_mask: " << d.count() << " ms\n";

        //tmp_timer = std::chrono::steady_clock::now();
        cv::Mat_<std::int32_t> mask_cell_label(lmask.size());
        cv::connectedComponents(lmask, mask_cell_label);
        // d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "connectedComponents: " << d.count() << " ms\n";
        
        // tmp_timer = std::chrono::steady_clock::now();
        {
            cv::Mat comp_img;
            mask_cell_label.convertTo(comp_img, CV_16U);
            // cv::imwrite("mask_label.tiff", comp_img);
            if(v_comp) {
                v_comp(comp_img);
            }
            if(v_mask) {
                v_mask(lmask);
            }
        }
		// d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "v_comp & v_mask: " << d.count() << " ms\n";
        // lmask = lmask / 255;
        // ip_convert(lmask,           CV_32F);
        ip_convert(mask_cell_label, CV_32F);
        ip_convert(sd,              CV_32F);
        ip_convert(mean,            CV_32F);
        ip_convert(cv,              CV_32F);
		// d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "lots of ip_convert: " << d.count() << " ms\n";
        
		// tmp_timer = std::chrono::steady_clock::now();
        cv::Mat_<std::uint16_t> margin = mat.clone();
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
        ObjMat<cv::Mat, std::int32_t> warped_mask(clhn, clwn);
        stat::Mats<Float> stat_mats(clhn, clwn);
        auto cell = warped_agg_mat.make_at_result();
        std::vector<decltype(cell)> mats;
		// d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "init stat_mat: " << d.count() << " ms\n";
        
		// tmp_timer = std::chrono::steady_clock::now();
        for(int i = 0; i < clhn; i ++) {
            for(int j = 0; j < clwn; j ++) {
                if(!warped_agg_mat.at_cell(
                    cell, i, j, cv::Size(1, 1)
                )) {
                    throw std::out_of_range(
                        fmt::format("invalid cell index ({},{})", i, j)
                    );
                }
                if(!warped_agg_mat.at_cell_all(
                    mats, i, j, cv::Size(clw_px, clh_px)
                )) {
                    throw std::out_of_range(
                        fmt::format("invalid cell index ({},{})", i, j)
                    );
                }
                auto label = cell.patch.at<float>(0, 0);
                std::int32_t int_label = std::round(label);
                auto& subpix_cent = mats.at(0).img_p;
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
                stat_mats.min_cv_pos(i, j) = min_cv_pos;

                if(v_margin){
                    cv::Point pb_img_tl(std::ceil(subpix_cent.x - clw_px/2.0),
                                        std::ceil(subpix_cent.y - clh_px/2.0));
                    cv::Point pb_swin_tl(pb_img_tl.x + min_cv_pos.x - swin_w_px/2,
                                         pb_img_tl.y + min_cv_pos.y - swin_h_px/2);
                    cv::Rect rect(pb_swin_tl.x, pb_swin_tl.y, swin_w_px, swin_h_px);
                    cv::rectangle(margin, rect, 65536/2);
                }

                ip_convert(sum_mask, CV_32F, 1.0 / 255);
                warped_mask     (i, j)  = sum_mask;
                mats.clear();
            }
        }
        // d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "calculate stat_mat: " << d.count() << " ms\n";

        if(v_margin){
            v_margin(margin);
        }
        margin.release();

        return nucleona::make_tuple(std::move(stat_mats), std::move(warped_mask));
    }
private:
    cv::Mat lab_to_mask(cv::Mat lab, std::int32_t i) const {
        return lab == i;
    }
    auto make_large_cv_mat(
        cv::Mat mat, int conv_w, int conv_h
    ) const {
        cv::Mat kern(conv_h, conv_w, type_to_depth<Float>());
        kern.setTo(1.0 / (conv_w * conv_h));

        cv::GMat g_in;
        auto g_x_mean = cv::gapi::filter2D(g_in, type_to_depth<Float>(), kern);
        auto g_x_mean_2 = cv::gapi::mul(g_x_mean, g_x_mean);
        auto g_x_2 = cv::gapi::mul(g_in, g_in);
        auto g_x_2_mean = cv::gapi::filter2D(g_x_2, type_to_depth<Float>(), kern);
        auto g_var = cv::gapi::sub(g_x_2_mean, g_x_mean_2);
        auto g_sd = cv::gapi::sqrt(g_var);
        cv::GComputation computation(cv::GIn(g_in), cv::GOut(g_x_mean, g_sd));

        cv::Mat x_mean, sd;
        computation.apply(cv::gin(mat), cv::gout(x_mean, sd), cv::compile_args(cv::gapi::imgproc::gpu::kernels()));
        return nucleona::make_tuple(std::move(x_mean), std::move(sd));
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
