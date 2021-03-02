#pragma once
#include <ChipImgProc/utils.h>
#include <opencv2/core/ocl.hpp>
#include <opencv2/gapi.hpp>
#include <opencv2/gapi/core.hpp>
#include <opencv2/gapi/imgproc.hpp>
#include <opencv2/gapi/gpu/imgproc.hpp>
namespace chipimgproc::warped_mat {

struct MakeMask {
    cv::Mat operator()(
        cv::Point origin, 
        int clw,    int clh,
        int clwd,   int clhd,
        int w,      int h,
        int conv_w, int conv_h,
        double um2px_r,
        int clwn,   int clhn,
        cv::Mat warpmat,
        cv::Size dsize
    ) const {
        // auto roiw = clwd * clwn;
        // auto roih = clhd * clhn;
        // int clwsp = (clwd - clw) / 2;
        // int clhsp = (clhd - clh) / 2;
        // int conv_w_px = std::round(conv_w * um2px_r);
        // int conv_h_px = std::round(conv_h * um2px_r);
        // cv::Mat res = cv::Mat::zeros(dsize, CV_8U);
        // cv::Mat kern = cv::Mat(conv_h_px, conv_w_px, CV_64F, cv::Scalar(1.0 / (conv_h_px * conv_w_px)));
        // cv::GMat g_in, g_out = cv::gapi::filter2D(g_in, CV_8U, kern);
        // cv::GComputation computation(cv::GIn(g_in), cv::GOut(g_out));
        // auto tmp_timer(std::chrono::steady_clock::now());
        // std::chrono::duration<double, std::milli> d1, d2, d3, d4, d5;
        // for(int i = 0; i < 4; i ++) {
        //     for(int j = 0; j < 4; j ++) {
        //         cv::Mat mat = cv::Mat::zeros(h, w, CV_8U);
        //         cv::Point org_off(
        //             origin.x + clwd * j,
        //             origin.y + clhd * i
        //         );
        //         {
        //             cv::Rect mat_rect(0, 0, mat.cols, mat.rows);
        //             auto tmp = mat(cv::Rect(org_off.x, org_off.y, roiw, roih) & mat_rect);
        //             partial_mask(
        //                 clw, clh, clwd * 2, clhd * 2, clwsp, clhsp, tmp
        //             );
        //         }
        //         cv::Mat wmat(dsize, CV_8U);
        //         tmp_timer = std::chrono::steady_clock::now();
        //         cv::warpAffine(mat, wmat, warpmat, dsize);
        //         d4 += std::chrono::steady_clock::now() - tmp_timer;
        //         mat.release();
        //         cv::Mat wcmat(dsize, CV_8U);
        //         tmp_timer = std::chrono::steady_clock::now();
        //         computation.apply(cv::gin(wmat), cv::gout(wcmat)/*, cv::compile_args(cv::gapi::imgproc::gpu::kernels())*/);
        //         d1 += std::chrono::steady_clock::now() - tmp_timer;
        //         res += wcmat;
        //         // cv::Mat tmp = filter2D(wmat, kern);
        //         // cv::Mat wcmat(tmp.size(), CV_8U);
        //         // tmp.convertTo(wcmat, CV_8U);
        //         // res += wcmat;
        //     }
        // }
        // std::cout << "make_mask-computation: " << d1.count() << " ms\n";
        // std::cout << "warpAffine-computation: " << d4.count() << " ms\n";
        // return res >= 1;
        // // return res;

        auto roiw = clwd * clwn;
        auto roih = clhd * clhn;
        int clwsp = (clwd - clw) / 2;
        int clhsp = (clhd - clh) / 2;
        int conv_w_px = std::round(conv_w * um2px_r);
        int conv_h_px = std::round(conv_h * um2px_r);
        cv::Mat res = cv::Mat::zeros(dsize, CV_8U);
        cv::Mat kern = cv::Mat(conv_h_px, conv_w_px, CV_64F, cv::Scalar(1.0 / (conv_h_px * conv_w_px)));
        cv::GMat g_in;
        cv::GMat g_tmp = cv::gapi::warpAffine(g_in, warpmat, dsize);
        cv::GMat g_out = cv::gapi::filter2D(g_tmp, CV_8U, kern);
        cv::GComputation computation(cv::GIn(g_in), cv::GOut(g_out));
        auto tmp_timer(std::chrono::steady_clock::now());
        std::chrono::duration<double, std::milli> d;
        for(int i = 0; i < 4; i ++) {
            for(int j = 0; j < 4; j ++) {
                cv::Mat mat = cv::Mat::zeros(h, w, CV_8U);
                cv::Point org_off(
                    origin.x + clwd * j,
                    origin.y + clhd * i
                );
                {
                    cv::Rect mat_rect(0, 0, mat.cols, mat.rows);
                    auto tmp = mat(cv::Rect(org_off.x, org_off.y, roiw, roih) & mat_rect);
                    partial_mask(
                        clw, clh, clwd * 2, clhd * 2, clwsp, clhsp, tmp
                    );
                }
                cv::Mat warp_mask(dsize, CV_8U);
                tmp_timer = std::chrono::steady_clock::now();
                computation.apply(cv::gin(mat), cv::gout(warp_mask)/*, cv::compile_args(cv::gapi::imgproc::gpu::kernels())*/);
                d += std::chrono::steady_clock::now() - tmp_timer;
                res += warp_mask;
                // cv::Mat tmp = filter2D(wmat, kern);
                // cv::Mat wcmat(tmp.size(), CV_8U);
                // tmp.convertTo(wcmat, CV_8U);
                // res += wcmat;
            }
        }
        std::cout << "make_mask-computation: " << d.count() << " ms\n";
        return res >= 1;
        // return res;
    } 
private:
    void partial_mask(
        int clw,  int clh,
        int clwd, int clhd,
        int clwsp, int clhsp,
        cv::Mat& mat
    ) const {
        for(int i = 0; i < mat.rows; i += clhd) {
            for(int j = 0; j < mat.cols; j += clwd) {
                cv::Rect clr(j + clwsp, i + clhsp, clw, clh);
                mat(clr).setTo(1);
            }
        }
    }
};


}
