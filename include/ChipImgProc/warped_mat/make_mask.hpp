#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc::warped_mat {

struct MakeMask {
    cv::Mat_<std::uint8_t> operator()(
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
        auto roiw = clwd * clwn;
        auto roih = clhd * clhn;
        int clwsp = (clwd - clw) / 2;
        int clhsp = (clhd - clh) / 2;
        int conv_w_px = std::round(conv_w * um2px_r);
        int conv_h_px = std::round(conv_h * um2px_r);
        cv::Mat_<std::uint8_t> res = cv::Mat_<std::uint8_t>::zeros(dsize);
        for(int i = 0; i < 2; i ++) {
            for(int j = 0; j < 2; j ++) {
                cv::Mat_<std::uint8_t> mat = cv::Mat_<std::uint8_t>::zeros(h, w);
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
                cv::Mat_<std::uint8_t> wmat(dsize);
                cv::warpAffine(mat, wmat, warpmat, dsize);
                mat.release();
                cv::Mat kern(conv_h_px, conv_w_px, CV_64F);
                kern.setTo(1.0 / (conv_h_px * conv_w_px));
                cv::Mat tmp = filter2D(wmat, kern);
                tmp -= 254.49;
                cv::Mat wcmat(tmp.size(), CV_8U);
                tmp.convertTo(wcmat, CV_8U);
                res += wcmat;
            }
        }
        return res >= 1;
        // return res;
    } 
private:
    void partial_mask(
        int clw,  int clh,
        int clwd, int clhd,
        int clwsp, int clhsp,
        cv::Mat_<std::uint8_t>& mat
    ) const {
        for(int i = 0; i < mat.rows; i += clhd) {
            for(int j = 0; j < mat.cols; j += clwd) {
                cv::Rect clr(j + clwsp, i + clhsp, clw, clh);
                mat(clr).setTo(255);
            }
        }
    }
};


}
