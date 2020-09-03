#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <string>
#include <istream>
#include <Nucleona/stream/null_buffer.hpp>
#include <Nucleona/tuple.hpp>
namespace chipimgproc{ namespace marker{

struct TxtToImg {
    auto operator()(
        const cv::Mat_<std::uint8_t>& mat,
        const cv::Mat_<std::uint8_t>& mask_cl,
        double cell_r_px,
        double cell_c_px,
        double border_px,
        std::ostream& log = nucleona::stream::null_out
    ) const {
        log << "mask_cl: " << std::endl;
        log << mask_cl << std::endl;
        cell_r_px *= 16;
        cell_c_px *= 16;
        border_px *= 16;

        int patch_r = std::round(cell_r_px);
        int patch_c = std::round(cell_c_px);
        int margin_px = std::round(border_px / 2);

        cv::Mat_<std::uint8_t> patch = cv::Mat_<std::uint8_t>::ones(patch_r, patch_c) * 255;
        cv::Mat_<std::uint8_t> block = cv::Mat_<std::uint8_t>::ones(patch_r + border_px, patch_c + border_px);
        cv::copyMakeBorder(
            patch, patch, 
            margin_px, margin_px, 
            margin_px, margin_px, 
            cv::BORDER_CONSTANT, 0
        );
        cv::Mat templ = chipimgproc::kron(mat,     patch);
        cv::Mat mask  = chipimgproc::kron(mask_cl, block);
        templ = affine_resize(templ, 0.0625, 0.0625, cv::INTER_AREA);
        mask = affine_resize(mask, 0.0625, 0.0625, cv::INTER_NEAREST);
        return nucleona::make_tuple(
            std::move(templ), 
            std::move(mask)
        );
    }
};
constexpr TxtToImg txt_to_img;
}}