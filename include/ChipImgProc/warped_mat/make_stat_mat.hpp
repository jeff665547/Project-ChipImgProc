/**
 * @file    make_stat_mat.hpp
 * @author  Chi-Hsuan Ho (jeffho@centrilliontech.com.tw)
 * @brief   @copybrief chipimgproc::warped_mat::MakeStatMat
 */
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
/**
 * @brief The MakeStatMat class is used to compute the extracted intensity from the fluorescent raw image of each probe.
 * 
 */
template<class Float>
struct MakeStatMat {
    /**
     * @brief       Compute the representative intensity for each probe.
     * @details     This algorithm first use the following procedure to generate the large warped mask 
     *              (Figure 4) to identify the pure probe convolution region of each probe in the 
     *              convolutional fluorescent images:
     *                  1. Generate the standard mask from the parameters users specified (origin, clw, 
     *                     clh, clwd, clhd, w, h, clwn, clhn). These parameters should be infered from 
     *                     the GDS spec.
     *                  2. Transform the generated standard mask to simulate the real probe position and 
     *                     its range for each probe through the given warpmat parameter (Figure 1).
     *                     @image html MakeStatisticsMatrix-affinetransform-concept.png       width=650px
     *                     Figure 1 Affine Transformation for simulating the real probe position.
     *                  3. Identify the pure probe convolution region for each probe under the convolution 
     *                     operation with the given small window (swin_w, swin_h, um2px_r) (Figure 2).
     *                     @image html MakeStatisticsMatrix-convolution-concept.png           width=650px
     *                     Figure 2 Convolution operation for identifying the pure signal region.
     *                  4. Remove the polluted area (pixels) for each probe to get the binarized mask 
     *                     representing the pure probe convolution region of each probe (Figure 3).
     *                     @image html MakeStatisticsMatrix-makewarpedmask-concept.png        width=650px
     *                     Figure 3 Binarized mask for removing the polluted area (pixels) for each probe.
     * 
     *              After that, this algorithm labels the pure probe convolution region of each probe 
     *              with the natural number (Figure 4). 
     *              @image html MakeStatisticsMatrix-largewarpedmask-concept.png   width=650px
     *              Figure 4 Large warped mask for indicating the pure probe convolution region of each probe.
     *              
     *              It then creates containers for storing the computed information of each probe. For more 
     *              details on extracting the desired inforamtion for each probe, please refer to 
     *              the procedure and the demonstrated image (Figure 5) below.
     *                  1. Get the label ID from the corresponding large warped mask and compute the 
     *                     subpixel-level information (positions, pure probe convolution region, 
     *                     ROI raw image) for that probe.
     *                  2. Remove the influence from other probes and the interpolation bias under the
     *                     subpixel-level domain by using the given theor_max_val.
     *                  3. Use the given small window (swin_w, swin_h) to compute the desired statistics for 
     *                     that window.
     *                  4. Stack up the above information and extract the intensity that is the most 
     *                     robust in this pure probe convolution region as the representative of that probe.
     *                  5. Store all the information for the downstream analysis and summary.
     *                     @image html MakeStatisticsMatrix-intensityextraction-concept.png   width=650px
     *                     Figure 5 Stack up all the useful information and extract the desired statistics that is the 
     *                     most robust in this pure probe convolution region of each probe.
     *              
     *              For more information, please refer to the source code.
     *              
     *              Examples:
     * 
     *              ChipImgProc: include/ChipImgProc/warped_mat.hpp:123
     * 
     * 
     * @param mat                Input fluorescent images.
     * @param origin             The origin of the coordinate system for the algorithm operation. It is usually set to (0, 0).
     * @param clw                The probe width in the rescaled um domain.
     * @param clh                The probe height in the rescaled um domain.
     * @param clwd               The probe width (includes an x-axis gap between two adjacent probes) in the rescaled um domain.
     * @param clhd               The probe height (includes a y-axis gap between two adjacent probes) in the rescaled um domain.
     * @param w                  The FOV width in the rescaled um domain.
     * @param h                  The FOV height in the rescaled um domain.
     * @param swin_w             The width (px domain) of the small window used to compute the statistics of the probe. It will 
     *                           be rounded up in the algorithm. 
     * @param swin_h             The height (px domain) of the small window used to compute the statistics of the probe. It will 
     *                           be rounded up in the algorithm.
     * @param um2px_r            um to pixel rate. The transformation rate for transforming the algorithm operating coordinate 
     *                           from um domain to pixel domain.
     * @param theor_max_val      Theoretical maximum value of the input image depth.
     * @param clwn               Number of cell (probe) in the row of an FOV.
     * @param clhn               Number of cell (probe) in the column of an FOV.
     * @param warpmat            The given transformation matrix that is used to inform the exact probe position in the 
     *                           fluorescent image.
     * @param v_margin           Debug viewer for outputting and viewing the final intensity-extracting area in the fluorescent 
     *                           images.
     * @param v_comp             Debug viewer for outputting and viewing the masking label (component) in the fluorescent image.
     * @param v_mask             Debug viewer for outputting and viewing the masking area in the fluorescent image.
     * 
     * @return auto A collection of computed statistics and other information (e.g. position, raw image) for each probe.

     */
    auto operator()(
        cv::Mat     mat,
        cv::Point2d     origin, 
        int clw,        int clh,
        int clwd,       int clhd,
        int w,          int h,
        double swin_w,  double swin_h,
        double          um2px_r,
        double          theor_max_val,
        int clwn,       int clhn,
        cv::Mat         warpmat,
        ViewerCallback  v_margin = {},
        ViewerCallback  v_comp   = {},
        ViewerCallback  v_mask   = {}
    ) const {
        
        /* Prepare parameters for the downstream analysis (the large waped mask generation and the probe intensity extraction) */
        int swin_w_px = std::round(swin_w * um2px_r);
        int swin_h_px = std::round(swin_h * um2px_r);
        int clw_px    = std::round(clw * um2px_r);
        int clh_px    = std::round(clh * um2px_r);

        // auto tmp_timer(std::chrono::steady_clock::now());
        // std::chrono::duration<double, std::milli> d, d1;

        /* Generate the large waped mask to identify the pure probe convolution region of each probe. */
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
        // chipimgproc::log.info("chipimgproc::warped_mat::MakeStatMat::operator()(...) - make_large_mask: {} ms", d.count());

        /* Label the pure probe convolution region of each probe with the natrual number. */
        // tmp_timer = std::chrono::steady_clock::now();
        cv::Mat_<std::int32_t> mask_cell_label(lmask.size());
        cv::connectedComponents(lmask, mask_cell_label);
        // d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "connectedComponents: " << d.count() << " ms\n";
        
        /* Convert the format of the labeled mask image (mask_cell_label) and output the 
           related debug images. */
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

        /* Stack up all the useful information above and creates containers (stat_mats, 
           cell_info) for storing the computed information of each probe. */
        ip_convert(mask_cell_label, CV_32F);
        cv::Mat mat_clone;
        mat.convertTo(mat_clone, CV_16U);
        auto warped_agg_mat = make_basic(warpmat, 
            std::vector<cv::Mat>({
                mask_cell_label,
                lmask,
                mat
            }),
            origin, clwd, clhd, w, h
        );
        ObjMat<RawPatch, std::int32_t> cell_info(clhn, clwn);
        stat::Mats<Float> stat_mats(clhn, clwn);
        auto cell = warped_agg_mat.make_at_result();
        std::vector<decltype(cell)> mats;
		// d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "init stat_mat: " << d.count() << " ms\n";
        
		// tmp_timer = std::chrono::steady_clock::now();

        /* Compute and extract the desired inforamtion for each probe. */
        for(int i = 0; i < clhn; i ++) {
            for(int j = 0; j < clwn; j ++) {
                /* Get the label ID from the corresponding large warped mask and compute the 
                   subpixel-level information (positions (cent_img, cent_rum), pure probe 
                   convolution region (sub_lab, sub_mask), ROI raw image (sub_raw)) for that 
                   probe. */
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
                if(!int_label) {
                    throw std::out_of_range(
                        fmt::format("invalid cell label, index ({},{}), label: {}", i, j, int_label)
                    );
                }
                auto& cent_img    = mats.at(0).img_p;
                auto& cent_rum    = mats.at(0).real_p;
                auto& sub_lab     = mats.at(0).patch;
                auto& sub_mask    = mats.at(1).patch;
                auto& sub_raw     = mats.at(2).patch;

                /* Remove the influence from other probes and the interpolation bias under
                   the subpixel-level domain. */
                cv::Mat int_sub_lab(sub_lab.size(), CV_32S);
                sub_lab -= 0.4;
                sub_lab.convertTo(int_sub_lab, CV_32S);
                sub_lab = lab_to_mask(int_sub_lab, int_label);
                sub_mask = sub_mask == 255;
                cv::Mat sum_mask = sub_mask & sub_lab;
				
                cv::threshold(sub_raw, sub_raw, theor_max_val, 0, cv::THRESH_TRUNC);
                // tmp_timer1 = std::chrono::steady_clock::now();

                /* Use the given small window (swin_w_px, swin_h_px) to compute the desired 
                   statistics (sub_mean, sub_mean_2, sub_var) for that probe.*/
                auto [sub_mean, sub_mean_2, sub_var] = make_cell_stats(sub_raw, theor_max_val, swin_w_px, swin_h_px);
                // d1 += std::chrono::steady_clock::now() - tmp_timer1;
                cv::Mat sub_cv_2           = sub_var / sub_mean_2;
                cv::patchNaNs(sub_cv_2, 0.0);
                
                /* Stack up the information, extract the intensity that is the most robust 
                   (min_cv_pos) in this pure probe convolution region as the representative
                   of this probe and store the information (stat_mats, cell_info) for the 
                   downstream analysis and summary. */
                cv::Point min_cv_pos; // pixel domain
                double min_cv_2;
                cv::minMaxLoc(sub_cv_2, &min_cv_2, nullptr, &min_cv_pos, nullptr, sum_mask);
                stat_mats.mean  (i, j)  = sub_mean.template at<Float>(min_cv_pos);
                stat_mats.stddev(i, j)  = std::sqrt(sub_var.template at<Float>(min_cv_pos));
                stat_mats.cv    (i, j)  = std::sqrt(min_cv_2);
                stat_mats.bg    (i, j)  = 0;
                stat_mats.num   (i, j)  = swin_w_px * swin_h_px;
                stat_mats.min_cv_pos(i, j) = min_cv_pos;

                /* Generate the mincv debug images. */
                if(v_margin){
                    cv::Point pb_img_tl(std::ceil(cent_img.x - clw_px/2.0),
                                        std::ceil(cent_img.y - clh_px/2.0));
                    cv::Point pb_swin_tl(pb_img_tl.x + min_cv_pos.x - swin_w_px/2,
                                         pb_img_tl.y + min_cv_pos.y - swin_h_px/2);
                    cv::Rect rect(pb_swin_tl.x, pb_swin_tl.y, swin_w_px, swin_h_px);
                    cv::rectangle(mat_clone, rect, 65536/2);
                }

                // ip_convert(sum_mask, CV_32F, 1.0 / 255);
                cell_info     (i, j)  = {sub_raw, cent_img, cent_rum};
                mats.clear();
            }
        }
        // d = std::chrono::steady_clock::now() - tmp_timer;
        // std::cout << "calculate stat_mat: " << d1.count() << " ms\n";
        // chipimgproc::log.info("chipimgproc::warped_mat::MakeStatMat::operator()(...) - calculate cell_mat: {} ms", d.count());

        /* Output the mincv debug images. */
        if(v_margin){
            v_margin(mat_clone);
        }

        /* Return a tuple of the created containers. */
        return nucleona::make_tuple(std::move(stat_mats), std::move(cell_info));
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
        computation.apply(cv::gin(mat), cv::gout(x_mean, sd));
        return nucleona::make_tuple(std::move(x_mean), std::move(sd));
    }
    auto make_cell_stats(
        cv::Mat mat, double theor_max_val, int conv_w, int conv_h
    ) const {

        cv::Mat x_mean   = mean(mat, conv_w, conv_h);
        trim_zero_maximum(x_mean, x_mean, theor_max_val);
        cv::Mat x_mean_2 = x_mean.mul(x_mean);
        cv::threshold(x_mean_2, x_mean_2, theor_max_val*theor_max_val, 0, cv::THRESH_TRUNC);
        auto    x_2      = mat.mul(mat);
        auto    x_2_mean = mean(x_2, conv_w, conv_h);
        trim_zero_maximum(x_2_mean, x_2_mean, theor_max_val*theor_max_val);
        cv::Mat var      = x_2_mean - x_mean_2;
        cv::threshold(var, var, 0, 0, cv::THRESH_TOZERO);

        return nucleona::make_tuple(
            std::move(x_mean),
            std::move(x_mean_2),
            std::move(var)
        );
    }
    void trim_zero_maximum(cv::InputArray src,cv::OutputArray dst, double theor_max_value) const {
        cv::threshold(src, dst, theor_max_value, 0, cv::THRESH_TRUNC);
        cv::threshold(dst, dst, 0, 0, cv::THRESH_TOZERO);
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
