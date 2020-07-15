#pragma once
#include <ChipImgProc/utils.h>
#include "reg_mat_helper.hpp"
#include <Nucleona/language.hpp>
#include <fmt/format.h>
#include <ChipImgProc/stat/cell.hpp>
#include <iostream>

namespace chipimgproc::warped_mat {

template<bool is_reg_mat = true>
struct Basic 
: public warped_mat::RegMatHelper<
    Basic<is_reg_mat>,
    is_reg_mat
> {
    using Base = warped_mat::RegMatHelper<
        Basic<is_reg_mat>,
        is_reg_mat
    >;

    template<class... RegMatArgs>
    Basic(
        cv::Mat                 warp_mat,
        std::vector<cv::Mat>    raw_images,
        RegMatArgs&&...         reg_mat_args
    ) 
    : Base          (FWD(reg_mat_args)...)
    , warp_mat_     (warp_mat)
    , raw_images_   (raw_images)
    {
        if(raw_images_.size() < 1) throw std::runtime_error("must provide at least 1 raw image");
        auto imsize = raw_images_[0].size();
        for(auto&& image : raw_images_) {
            if(imsize != image.size()) {
                throw std::runtime_error("all image must have same size");
            }
        }
    }
    std::vector<cv::Mat> at_real(
        double r, 
        double c, 
        cv::Size patch_size = cv::Size(5, 5)
    ) const {
        auto px_point = point_transform(c, r);
        if(!is_include_impl(px_point, patch_size)) {
            throw std::runtime_error(point_out_of_boundary(r, c, px_point));
        }
        std::vector<cv::Mat> res;
        for(auto&& raw_image : raw_images_) {
            cv::Mat img_roi(patch_size, raw_image.type());
            cv::getRectSubPix(raw_image, patch_size, px_point, img_roi);
            res.push_back(img_roi);
        }
        return res;
    }
    cv::Mat at_real(
        double r, double c, int i, 
        cv::Size patch_size = cv::Size(5, 5)
    ) const {
        auto px_point = point_transform(c, r);
        if(!is_include_impl(px_point, patch_size)) {
            throw std::runtime_error(point_out_of_boundary(r, c, px_point));
        }
        auto& raw_image = raw_images_.at(i);
        cv::Mat img_roi(patch_size, raw_image.type());
        cv::getRectSubPix(raw_image, patch_size, px_point, img_roi);
        return img_roi;
    }
private:
    auto is_include_impl(cv::Point2d px_point, cv::Size patch_size) const {
        auto safe_padding_x = patch_size.width * 2;
        auto safe_padding_y = patch_size.height * 2;
        if(px_point.x < safe_padding_x) return false;
        if(px_point.y < safe_padding_y) return false;
        if(px_point.x >= (raw_images_[0].cols - safe_padding_x)) return false;
        if(px_point.y >= (raw_images_[0].rows - safe_padding_y)) return false;
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
    cv::Mat                 warp_mat_    ;
    std::vector<cv::Mat>    raw_images_  ;
};
constexpr struct MakeBasic {
    auto operator()(
        cv::Mat                 warp_mat,
        std::vector<cv::Mat>    raw_images,
        cv::Point2d             origin,
        double                  xd, 
        double                  yd
    ) const {
        return Basic<true>(
            warp_mat, raw_images, origin,
            xd, yd
        );
    }
    auto operator()(
        cv::Mat                 warp_mat,
        std::vector<cv::Mat>    raw_images,
        cv::Point2d             origin,
        double                  cell_w, 
        double                  cell_h,
        double                  space_w, 
        double                  space_h 
    ) const {
        cv::Point2d _origin(
            origin.x + (space_w / 2),
            origin.y + (space_h / 2)
        );
        return Basic<true>(
            warp_mat, raw_images, 
            _origin,
            cell_w + space_w, 
            cell_h + space_h
        );
    }
    auto operator()(
        cv::Mat                warp_mat,
        std::vector<cv::Mat>   raw_images
    ) const {
        return Basic<false>(
            warp_mat, raw_images
        );
    }

} make_basic;
}