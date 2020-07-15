#pragma once
#include "warped_mat/basic.hpp"
#include "warped_mat/make_stat_mat.hpp"
#include "warped_mat/stat_reg_mat_helper.hpp"
#include "warped_mat/patch.hpp"
namespace chipimgproc {

constexpr auto make_basic_warped_mat = warped_mat::make_basic;

template<bool is_reg_mat = true>
using BasicWarpedMat = warped_mat::Basic<is_reg_mat>;

template<bool is_reg_mat>
struct WarpedMat 
: public warped_mat::StatRegMatHelper<
    WarpedMat<is_reg_mat>,
    is_reg_mat
>
{
    constexpr static bool support_reg_mat = is_reg_mat;

    using Base = warped_mat::StatRegMatHelper<
        WarpedMat<is_reg_mat>,
        is_reg_mat
    >;

    template<class... RegMatArgs>
    WarpedMat(
        cv::Mat             warp_mat,
        cv::Mat             raw_image,
        RegMatArgs&&...     reg_mat_args
    )
    : Base              (FWD(reg_mat_args)...)
    , basic_warped_mat_ (
        warp_mat, 
        { raw_image }
    )
    {}

    auto at_real(double r, double c, cv::Size patch_size = cv::Size(5, 5)) const {
        auto pxs = basic_warped_mat_.at_real(r, c, 0, patch_size);
        return warped_mat::Patch(pxs);
    }
private:
    warped_mat::Basic<false>  basic_warped_mat_   ;
};

constexpr struct MakeWarpedMat {
    using CellMasks = ObjMat<cv::Mat, std::uint32_t>;
    auto operator()(
        cv::Mat                 warp_mat,
        cv::Mat                 raw_image,
        stat::Mats<double>&&    stat_mats,
        CellMasks         &&    cell_mask,
        cv::Point2d             origin,
        double                  xd, 
        double                  yd
    ) const {
        return WarpedMat<true>(
            warp_mat, raw_image, 
            std::move(stat_mats), 
            std::move(cell_mask),
            origin,
            xd, yd
        );
    }
    auto operator()(
        cv::Mat                warp_mat,
        cv::Mat                raw_images
    ) const {
        return WarpedMat<false>(
            warp_mat, raw_images
        );
    }

} make_warped_mat;

}