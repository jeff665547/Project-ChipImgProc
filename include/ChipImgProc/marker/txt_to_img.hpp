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
        double cell_r,
        double cell_c,
        double border,
        double scale = 1.0,
        double usamp = 8.0,
        std::ostream& log = nucleona::stream::null_out
    ) const {
        log << "mask_cl: " << std::endl;
        log << mask_cl << std::endl;
        cell_r *= usamp;
        cell_c *= usamp;
        border *= usamp;

        int patch_r = std::round(cell_r);
        int patch_c = std::round(cell_c);
        int margin_px = std::round(border / 2);

        cv::Mat_<std::uint8_t> patch = cv::Mat_<std::uint8_t>::ones(patch_r, patch_c) * 255;
        cv::Mat_<std::uint8_t> block = cv::Mat_<std::uint8_t>::ones(patch_r + border, patch_c + border);
        cv::copyMakeBorder(
            patch, patch, 
            margin_px, margin_px, 
            margin_px, margin_px, 
            cv::BORDER_CONSTANT, 0
        );
        cv::Mat templ = chipimgproc::kron(mat,     patch);
        cv::Mat mask  = chipimgproc::kron(mask_cl, block);
        templ = affine_resize(templ, scale / usamp, 0, cv::INTER_AREA);
        mask = affine_resize(mask, scale / usamp, 0, cv::INTER_NEAREST);
        return nucleona::make_tuple(
            std::move(templ), 
            std::move(mask)
        );
    }
};
constexpr TxtToImg txt_to_img;
}}