#pragma once
#include <ChipImgProc/utils.h>
#include <opencv2/core/mat.hpp>
#include <cmath>

namespace chipimgproc::warped_mat {
namespace cimp = chipimgproc;
struct RescaleWarpMat {
    cv::Mat operator()(
        cv::Mat warp_mat, 
        const double& GDS_to_std, 
        const double& GDS_to_new_domain
    ) const {
        auto new_factor = GDS_to_new_domain/GDS_to_std; 
        cimp::typed_mat(warp_mat, [&new_factor](auto& mat){
            mat(0, 0) *= new_factor; mat(0, 1) *= new_factor;
            mat(1, 0) *= new_factor; mat(1, 1) *= new_factor;
        });
        return warp_mat;
    }
    cv::Mat operator()(
        const double& rescale_factor
    ) const {
        return cv::Mat::eye(2, 3, CV_32F)*rescale_factor;
    }
    cv::Mat operator()(
        const double& GDS_to_std,
        const double& GDS_to_new_domain
    ) const {
        auto new_factor = GDS_to_new_domain/GDS_to_std;
        return this->operator()(
            new_factor
        );
    }

};
constexpr RescaleWarpMat rescale_warp_mat;

}
