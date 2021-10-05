#pragma once
#include <ChipImgProc/utils.h>
#include <cmath>
namespace chipimgproc::warp_mat {
struct CreateWarp {
    auto operator()(
        double rot_deg, 
        double scale_x,     double scale_y, 
        double translate_x, double translate_y,
        double shear_x = 0, double shear_y = 0 
    ) const {
        double rad = rot_deg * PI / 180.0;
        cv::Mat warp_mat(2, 3, CV_64FC1, cv::Scalar(0));
        warp_mat.at<double>(0, 0) = scale_x*(std::cos(rad) - shear_y*std::sin(rad));
        warp_mat.at<double>(0, 1) = scale_x*(shear_x*std::cos(rad) - std::sin(rad));
        warp_mat.at<double>(0, 2) = translate_x;
        warp_mat.at<double>(1, 0) = scale_y*(std::sin(rad) + shear_y*std::cos(rad));
        warp_mat.at<double>(1, 1) = scale_y*(shear_x*std::sin(rad) + std::cos(rad));
        warp_mat.at<double>(1, 2) = translate_y;
        return warp_mat;
    }
};
constexpr CreateWarp create_warp;

}