#pragma once
#include "detection/mk_region.hpp"
#include <ChipImgProc/utils.h>
namespace chipimgproc::marker {

constexpr struct View {
    auto operator()(
        const cv::Mat_<std::uint16_t>& m, 
        const std::vector<detection::MKRegion>& mk_regs
    ) const {
        auto v = viewable(m);
        for(auto& mk_r : mk_regs) {
            cv::rectangle(v, mk_r, 32768, 1);
        }
        return v;
    }
} view;

}