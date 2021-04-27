#pragma once
#include "random_based.hpp"
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/marker/txt_to_img.hpp>
namespace chipimgproc::marker::detection {

struct Random : public RandomBased<Random> {
    using Base = RandomBased<Random>;
    Random(
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const std::int32_t&             pyramid_level,
        const double&                   theor_max_val,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius
    )
    : RandomBased(
        templ, mask, pyramid_level, theor_max_val,
        nms_count, nms_radius
    )
    {
        anchors_.emplace_back(
            (templ.cols * 0.5) - 0.5, 
            (templ.rows * 0.5) - 0.5
        );
    }
};

struct MakeRandom {
    auto operator()(
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const std::int32_t&             pyramid_level,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius,
        const double&                   theor_max_val
    ) const {
        return Random(
            templ, mask,
            pyramid_level,
            theor_max_val,
            nms_count,
            nms_radius
        );
    }
    auto operator()(
        const std::string&      pat_path,
        const double&           cell_r_px,
        const double&           cell_c_px,
        const double&           border_px,
        const std::int32_t&     pyramid_level,
        const double&           theor_max_val,
        const std::int32_t&     nms_count,
        const double&           nms_radius
    ) const {
        std::ifstream marker_in(pat_path);
        auto [templ, mask] = Loader::from_txt(marker_in);
        auto [templ_img, mask_img] = txt_to_img(templ, mask,
            cell_r_px,
            cell_c_px,
            border_px
        );
        return Random(
            templ_img, mask_img,
            pyramid_level,
            theor_max_val,
            nms_count,
            nms_radius
        );
    }
};
constexpr MakeRandom make_random;
}