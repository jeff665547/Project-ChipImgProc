#pragma once
#include "detection/mk_region.hpp"
#include <ChipImgProc/utils.h>
namespace chipimgproc::marker {

constexpr struct View {
    auto operator()(
        const cv::Mat& m, 
        const std::vector<detection::MKRegion>& mk_regs,
        const cv::Scalar& color = 32768
    ) const {
        auto v = viewable(m);
        for(auto& mk_r : mk_regs) {
            cv::rectangle(v, mk_r, color, 1);
        }
        return v;
    }
    auto operator() (
        const cv::Mat& m,
        const std::vector<cv::Point>& pos,
        const cv::Scalar& color = 32768
    ) const {
        auto v = viewable(m);
        for(auto& p : pos) {
            cv::drawMarker(v, p, color, cv::MARKER_CROSS, 20, 1);
        }
        return v;

    }
} view;

}