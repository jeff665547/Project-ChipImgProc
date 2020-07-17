#pragma once
#include <ChipImgProc/utils.h>
#include <cmath>
namespace chipimgproc::rotation {
struct FromWarpMat {
    double operator()(cv::Mat warp_mat) const {
        double res;
        typed_mat(warp_mat, [&res](auto&& mat){
            auto rad = std::atan(mat(1, 0) / mat(0, 0));
            res = rad * 180 / PI;
        });
        return res;
    }
};

constexpr FromWarpMat from_warp_mat;

} // namespace chipimgproc::rotation
