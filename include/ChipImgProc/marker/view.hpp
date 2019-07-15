#pragma once
#include "detection/mk_region.hpp"
#include <ChipImgProc/utils.h>
namespace chipimgproc::marker {

constexpr struct View {
    auto operator()(
        const cv::Mat& m, 
        const std::vector<detection::MKRegion>& mk_regs
    ) const {
        auto v = viewable(m);
        for(auto& mk_r : mk_regs) {
            cv::rectangle(v, mk_r, cmax(m) / 2, 1);
        }
        return v;
    }
} view;

}