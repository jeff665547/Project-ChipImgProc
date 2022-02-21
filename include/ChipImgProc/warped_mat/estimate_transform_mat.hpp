/**
 * @file    estimate_transform_mat.hpp
 * @author  Chi-Hsuan Ho (jeffho@centrilliontech.com.tw)
 * @brief   @copybrief chipimgproc::warped_mat::EstimateTransformMat
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <opencv2/calib3d.hpp>

namespace chipimgproc::warped_mat {
/**
 * @brief The EstimateTransformMat class is used to estimate the transformation matrix from the given point sets.
 * 
 */
struct EstimateTransformMat {
    /**
     * @brief       Estimate the transformation matrix from the input point sets.
     * @details     This algorithm filters the unreasonable marker position and 
     *              its corresponding theoretical marker position pair by comparing 
     *              the relationship among the points in the source set and that in 
     *              the destination set. Each member of the following source (src) 
     *              set should match the corresponding member in the destination 
     *              (dst) set. The element order must be the same and elements must 
     *              be paired in these two point collections (src, dst).
     * 
     *              For more information, please refer to the source code.
     * 
     *              Examples:
     * 
     *              Summit.Grid: <a href="http://gitlab.centrilliontech.com.tw:10088/centrillion/Summit.Grid/blob/1.3.x/include/summit/app/grid/pipeline/chip.hpp#L832">include/summit/app/grid/pipeline/chip.hpp: 832</a> 
     * 
     * @param src       A collection of the marker positions found by other algorithms.
     * @param dst       A collection of the theoretical marker positions defined according to the GDS file.
     * @return cv::Mat  A transformation matrix corresponding to the input point sets.
     */
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
                dst[i].x, dst[i].y
                // dst[i].x - 0.5, dst[i].y - 0.5 // FIXME: unsure -0.5 problem need to be solved or not
            );
        }
        // std::cout << _src << '\n';
        // std::cout << _dst << '\n';
        return cv::estimateAffinePartial2D(_src, _dst);
    }
};

constexpr EstimateTransformMat estimate_transform_mat;

} // namespace chipimgproc::warped_mat
