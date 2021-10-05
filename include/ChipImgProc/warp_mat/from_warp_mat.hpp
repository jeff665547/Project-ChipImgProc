#pragma once
#include <ChipImgProc/utils.h>
#include <cmath>
namespace chipimgproc::warp_mat {
struct RotDegfromWarp {
    double operator()(cv::Mat warp_mat) const {
        double res;
        typed_mat(warp_mat, [&res](auto&& mat){
            auto rad = std::atan(mat(1, 0) / mat(0, 0));
            res = rad * 180 / PI;
        });
        return res;
    }
};
constexpr RotDegfromWarp rot_deg_from_warp;

struct SlopefromWarp {
    double operator()(cv::Mat warp_mat) const {
        double res;
        typed_mat(warp_mat, [&res](auto&& mat){
            res = mat(1, 0) / mat(0, 0);
        });
        return res;
    }
};
constexpr SlopefromWarp slope_from_warp;

struct ScalefromWarp {
    auto operator()(cv::Mat warp_mat) const {
        cv::Point2d res;
        typed_mat(warp_mat, [&res](auto&& mat){
            res.x = std::sqrt(std::pow(mat(0, 0), 2.0) + std::pow(mat(0, 1), 2.0));
            res.y = std::sqrt(std::pow(mat(1, 0), 2.0) + std::pow(mat(1, 1), 2.0));
        });
        return res;
    }
};
constexpr ScalefromWarp scale_from_warp;

}
