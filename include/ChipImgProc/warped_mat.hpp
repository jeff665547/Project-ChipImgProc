#pragma once
#include <ChipImgProc/utils.h>
#include "warped_mat/reg_mat_helper.hpp"
#include <Nucleona/language.hpp>
#include <fmt/format.h>
#include <ChipImgProc/stat/cell.hpp>
#include <iostream>

namespace chipimgproc {
struct WarpedMatPatch {
    cv::Mat             patch;
    stat::Cell<double>  stat;
};
template<class ImgPX = std::uint16_t, bool is_reg_mat = true>
struct WarpedMat 
: public wraped_mat::RegMatHelper<
    WarpedMat<ImgPX, is_reg_mat>,
    is_reg_mat
> {
    using Base = wraped_mat::RegMatHelper<
        WarpedMat<ImgPX, is_reg_mat>,
        is_reg_mat
    >;

    template<class... RegMatArgs>
    WarpedMat(
        cv::Mat         warp_mat,
        cv::Mat_<ImgPX> raw_image,
        RegMatArgs&&... reg_mat_args
    ) 
    : warp_mat_     (warp_mat)
    , raw_image_    (raw_image)
    , Base          (FWD(reg_mat_args)...)
    {}
    auto at_real(
        double r, 
        double c, 
        cv::Size patch_size = cv::Size(5, 5)
    ) const {
        auto px_point = point_transform(c, r);
        auto safe_padding_x = patch_size.width * 2;
        auto safe_padding_y = patch_size.height * 2;
        if(!is_include_impl(px_point, patch_size)) {
            throw std::runtime_error(point_out_of_boundary(r, c, px_point));
        }
        cv::Mat res(patch_size, raw_image_.type());
        cv::getRectSubPix(raw_image_, patch_size, px_point, res);
        auto stat = stat::Cell<double>::make(res);
        return WarpedMatPatch {
            res, 
            std::move(stat)
        };
    }
private:
    auto is_include_impl(cv::Point2d px_point, cv::Size patch_size) const {
        auto safe_padding_x = patch_size.width * 2;
        auto safe_padding_y = patch_size.height * 2;
        if(px_point.x < safe_padding_x) return false;
        if(px_point.y < safe_padding_y) return false;
        if(px_point.x >= (raw_image_.cols - safe_padding_x)) return false;
        if(px_point.y >= (raw_image_.rows - safe_padding_y)) return false;
        return true;
    }
    cv::Point2d point_transform(double x, double y) const {
        std::vector<cv::Vec2d> src(1);
        std::vector<cv::Vec2d> dst(1);
        src[0][0] = x;
        src[0][1] = y;
        cv::transform(src, dst, warp_mat_);
        return cv::Point2d(dst.at(0)[0], dst.at(0)[1]);
    }
    static std::string point_out_of_boundary(double r, double c, const cv::Point2d& px) {
        return fmt::format(
            "point out of boundary, real: ({}, {}), pixel: ({}, {})",
            c, r, px.x, px.y
        );
    }
    cv::Mat             warp_mat_   ;
    cv::Mat_<ImgPX>     raw_image_  ;
};
constexpr struct MakeWarpedMat {
    template<class ImgPX>
    auto operator()(
        cv::Mat             warp_mat,
        cv::Mat_<ImgPX>     raw_image,
        cv::Point2d         origin,
        double              xd, 
        double              yd
    ) const {
        return WarpedMat<ImgPX, true>(
            warp_mat, raw_image, origin,
            xd, yd
        );
    }
    template<class ImgPX>
    auto operator()(
        cv::Mat             warp_mat,
        cv::Mat_<ImgPX>     raw_image
    ) const {
        return WarpedMat<ImgPX, false>(
            warp_mat, raw_image
        );
    }

} make_warped_mat;
}