#pragma once
#include <ChipImgProc/multi_tiled_mat.hpp>
#include <ChipImgProc/points.hpp>
#include <vector>
#include <ChipImgProc/utils.h>
#include <algorithm>
#include <ChipImgProc/wrapper/acc_bind.hpp>
namespace chipimgproc{ namespace analysis{

constexpr struct ProbeSort {
    template<class MAT>
    auto operator() (const MAT& mat ) const {
        auto&& mean = wrapper::bind_acc(mat, nucleona::copy(MAT::min_cv_mean));
        auto v_points = make_points<cv::Point>(mat);
        std::vector<cv::Point> points(v_points.begin(), v_points.end());
        
        std::cout << __FILE__ << ":" << __LINE__ 
            << ", points.size(): " << points.size() << std::endl;

        std::sort(points.begin(), points.end(), [&mean](
            const cv::Point& p0,
            const cv::Point& p1
        ){
            return mean(p0.y, p0.x) < mean(p1.y, p1.x);
        });

        return points;
    }
} probe_sort;

}}