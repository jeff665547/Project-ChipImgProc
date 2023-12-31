#pragma once
#include "warped_mat/basic.hpp"
#include "warped_mat/make_stat_mat.hpp"
#include "warped_mat/stat_reg_mat_helper.hpp"
#include "warped_mat/patch.hpp"
namespace chipimgproc {

constexpr auto make_basic_warped_mat = warped_mat::make_basic;

template<bool is_reg_mat = true>
using BasicWarpedMat = warped_mat::Basic<is_reg_mat>;

template<bool is_reg_mat = true, class Float = float>
struct WarpedMat 
: public warped_mat::StatRegMatHelper<
    WarpedMat<is_reg_mat, Float>,
    is_reg_mat,
    Float,
    warped_mat::Patch
>
{
    constexpr static bool support_reg_mat = is_reg_mat;

    using Base = warped_mat::StatRegMatHelper<
        WarpedMat<is_reg_mat>,
        is_reg_mat,
        Float,
        warped_mat::Patch
    >;
    using AtResult = warped_mat::Patch;

    WarpedMat() = default;

    template<class... RegMatArgs>
    WarpedMat(
        cv::Mat             warp_mat,
        cv::Mat             raw_image,
        double              max_x,
        double              max_y,
        RegMatArgs&&...     reg_mat_args
    )
    : Base              (FWD(reg_mat_args)..., max_x, max_y)
    , basic_warped_mat_ (
        warp_mat, 
        { raw_image },
        max_x, max_y
    )
    {}

    // (*)
    // bool at_real_pos(warped_mat::Patch& res, double r, double c) const {
    //     if(!basic_warped_mat_.at_real_pos(res, r, c)) {
    //         return false;
    //     }
    //     return true;
    // }
    bool at_real(warped_mat::Patch& res, double r, double c, cv::Size patch_size = cv::Size(5, 5)) const {
        auto basic_patch = basic_warped_mat_.make_at_result();
        if(!basic_warped_mat_.at_real(basic_patch, r, c, 0, patch_size)) {
            return false;
        }
        res = warped_mat::Patch(std::move(basic_patch));
        return true;
    }
    const cv::Mat& warp_mat() const {
        return basic_warped_mat_.warp_mat();
    }
    static warped_mat::Patch make_at_result() {
        return warped_mat::Patch{};
    }
private:
    warped_mat::Basic<false>  basic_warped_mat_   ;
};

constexpr struct MakeWarpedMat {
    using CellMasks = ObjMat<cv::Mat, std::int32_t>;
    template<class Float>
    auto operator()(
        cv::Mat                 warp_mat,
        cv::Mat                 raw_image,
        double                  x_max, 
        double                  y_max,
        stat::Mats<Float>&&     stat_mats,
        CellMasks        &&     cell_mask,
        cv::Point2d             origin,
        double                  xd, 
        double                  yd
    ) const {
        return WarpedMat<true, Float>(
            warp_mat, raw_image, 
            x_max, y_max,
            std::move(stat_mats), 
            std::move(cell_mask),
            origin,
            xd, yd
        );
    }
    auto operator()(
        cv::Mat                warp_mat,
        cv::Mat                raw_images,
        double                 max_x = std::numeric_limits<double>::max(),
        double                 max_y = std::numeric_limits<double>::max()
    ) const {
        return WarpedMat<false>(
            warp_mat, raw_images, max_x, max_y
        );
    }
    auto operator()(
        cv::Mat     warp_mat,
        cv::Mat     mat,
        // all micron level below
        cv::Point2d origin, 
        int clw,         int clh,
        int clwd,        int clhd,
        int w,           int h,
        double swin_w,   double swin_h,
        double um2px_r,  double theor_max_val,
        int clwn,        int clhn,
        ViewerCallback v_margin
    ) const {
        auto tmp_timer(std::chrono::steady_clock::now());
        std::chrono::duration<double, std::milli> d;
        warped_mat::MakeStatMat<float> make_stat_mat;
        auto [stat_mats, center_info] = make_stat_mat(
            mat, origin, 
            clw,    clh,
            clwd,   clhd,
            w,      h,
            swin_w, swin_h,
            um2px_r,
            theor_max_val,
            clwn,   clhn,
            warp_mat,
            v_margin
        );
        d = std::chrono::steady_clock::now() - tmp_timer;
        chipimgproc::log.info("ChipImgProc::MakeWarpedMat::operator()(...) - make_stat_mat: {} ms", d.count());

        return WarpedMat<true, float>(
            warp_mat, mat, 
            w, h,
            std::move(stat_mats), 
            std::move(center_info),
            origin,
            clwd, clhd
        );
    }
    auto operator()(
        cv::Mat     warp_mat,
        cv::Mat     mat,
        // all micron level below
        cv::Point2d origin, 
        int clw,    int clh,
        int clwd,   int clhd,
        int w,      int h,
        double win_r,
        double um2px_r,
        double theor_max_val,
        int clwn,   int clhn,
        ViewerCallback v_margin
    ) const {
        // int swin_w = std::round(clw * win_r);
        // int swin_h = std::round(clh * win_r);
        auto swin_w = clw * win_r;
        auto swin_h = clh * win_r;
        if(swin_w < 3 || swin_h < 3) throw std::invalid_argument("win_r too small, unable to generate proper filter");
        return operator()(warp_mat, mat, origin, clw, clh, clwd, 
            clhd, w, h, swin_w, swin_h, um2px_r, theor_max_val, 
            clwn, clhn, v_margin);
    }
} make_warped_mat;

}