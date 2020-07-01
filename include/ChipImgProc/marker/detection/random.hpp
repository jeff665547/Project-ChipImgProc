#pragma once
#include "random_based.hpp"
namespace chipimgproc::marker::detection {

struct Random : public RandomBased<Random> {
    using Base = RandomBased<Random>;
    Random(
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const std::int32_t&             pyramid_level,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius
    )
    : RandomBased(
        templ, mask, pyramid_level, 
        nms_count, nms_radius
    )
    {
        anchors_.emplace_back(
            (templ.cols * 0.5) - 0.5, 
            (templ.rows * 0.5) - 0.5
        );
    }
};

}