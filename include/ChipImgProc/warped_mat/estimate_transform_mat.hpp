#pragma once
#include <ChipImgProc/utils.h>
#include <opencv2/calib3d.hpp>

namespace chipimgproc::warped_mat {
struct EstimateTransformMat {
    template<class Src, class Dst>
    cv::Mat operator()(Src&& src, Dst&& dst) const {
        std::vector<cv::Point2d> _src;
        std::vector<cv::Point2d> _dst;

        auto n = std::distance(src.begin(), src.end());
        auto m = std::distance(dst.begin(), dst.end());
        if(n != m) {
            throw std::runtime_error("src and dst points number not match");
        }
        for(size_t i = 0; i < n; i ++) {
            _src.emplace_back(
                src[i].x - 0.5, src[i].y - 0.5
            );
            _dst.emplace_back(
                dst[i].x - 0.5, dst[i].y - 0.5
            );
        }
        return cv::estimateAffine2D(_src, _dst);
    }
};

constexpr EstimateTransformMat estimate_transform_mat;

} // namespace chipimgproc::warped_mat
