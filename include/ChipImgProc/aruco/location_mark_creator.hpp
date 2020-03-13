#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include "utils.hpp"

namespace chipimgproc::aruco {

constexpr struct LocationMarkCreator {
    auto operator()(
        const std::int32_t outer_width
      , const std::int32_t inner_width
      , const std::int32_t padding
      , const std::int32_t margin
      , double scale = 1.0
    ) const {
        using matrix_t = cv::Mat_<std::uint8_t>;

        std::int32_t m = margin, b = (outer_width - inner_width) / 2;
        matrix_t templ = matrix_t::zeros(inner_width, inner_width);
        cv::copyMakeBorder(templ, templ, b, b, b, b, cv::BORDER_CONSTANT, 255);
        cv::copyMakeBorder(templ, templ, m, m, m, m, cv::BORDER_CONSTANT, 0);
        chipimgproc::aruco::Utils::resize(templ, templ, scale, scale);

        b += padding + margin;
        std::int32_t blank_width = inner_width - padding * 2;
        matrix_t mask = matrix_t::zeros(blank_width, blank_width);
        cv::copyMakeBorder(mask, mask, b, b, b, b, cv::BORDER_CONSTANT, 255);
        chipimgproc::aruco::Utils::resize(mask, mask, scale, scale, cv::INTER_NEAREST);

        return std::make_tuple(templ, mask);
    }
}
create_location_marker;

}