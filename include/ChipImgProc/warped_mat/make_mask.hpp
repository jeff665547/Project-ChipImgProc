#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc::warped_mat {

struct MakeMask {
    cv::Mat_<std::uint8_t> operator()(
        cv::Point origin, 
        int clw,    int clh,
        int clwd,   int clhd,
        int clwn,   int clhn,
        int w,      int h,
        int conv_w, int conv_h,
        cv::Mat warpmat
    ) const {
        std::vector<cv::Mat_<std::uint8_t>> mat_vec;
        for(int i = 0; i < 2; i ++) {
            for(int j = 0; j < 2; j ++) {
                cv::Mat_<std::uint8_t> mat = cv::Mat_<std::uint8_t>::zeros(h, w);
                cv::Point org_off(
                    origin.x + clwd * j,
                    origin.y + clhd * i
                );
                auto roiw = clwd * (clwn - 1) + clw;
                auto roih = clhd * (clhn - 1) + clh;
                {
                    auto tmp = mat(cv::Rect(org_off.x, org_off.y, roiw, roih));
                    partial_mask(
                        clw, clh, clwd * 2, clhd * 2, tmp
                    );
                }
                cv::Mat_<std::uint8_t> wmat(mat.size());
                cv::warpAffine(mat, wmat, warpmat, wmat.size());
                mat.release();
                cv::Mat kern(conv_h, conv_w, CV_64F);
                kern.setTo(1.0 / (conv_h * conv_w));
                cv::Mat tmp = filter2D(wmat, kern);
                cv::Mat wcmat(tmp.size(), CV_8U);
                tmp.convertTo(wcmat, CV_8U);
                mat_vec.push_back(wcmat);
            }
        }
        cv::Mat_<std::uint8_t> res(mat_vec[0].size());
        res.setTo(0);
        for(std::size_t i = 0; i < mat_vec.size(); i ++) {
            auto& mat = mat_vec.at(i);
            res += mat;
        }
        return res >= 1;
    } 
private:
    void partial_mask(
        int clw,  int clh,
        int clwd, int clhd,
        cv::Mat_<std::uint8_t>& mat
    ) const {
        for(int i = 0; i < mat.rows; i += clhd) {
            for(int j = 0; j < mat.cols; j += clwd) {
                cv::Rect clr(j, i, clw, clh);
                mat(clr).setTo(1);
            }
        }
    }
};


}
