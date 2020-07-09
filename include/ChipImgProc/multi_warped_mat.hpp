#pragma once
#include "warped_mat.hpp"
namespace chipimgproc {

template<class ImgPX = std::uint16_t, bool is_reg_mat = true>
struct MultiWarpedMat 
: public wraped_mat::RegMatHelper<
    MultiWarpedMat<ImgPX, is_reg_mat>,
    is_reg_mat
>
{
    using FOV = WarpedMat<ImgPX, is_reg_mat>;
    using Base = wraped_mat::RegMatHelper<
        MultiWarpedMat<ImgPX, is_reg_mat>,
        is_reg_mat
    >;

    template<class... RegMatArgs>
    MultiWarpedMat(
        std::vector<FOV>&&          mats,
        std::vector<cv::Point2d>&&  st_pts,
        RegMatArgs&&...             reg_mat_args
    )
    : Base          (FWD(reg_mat_args)...)
    , mats_         (std::move(mats))
    , st_pts_       (std::move(st_pts))
    {
        if(mats_.size() != st_pts_.size()) {
            throw std::runtime_error(
                "stitching point number must match matrix number"
            );
        }
    }
    auto at_real(double r, double c, cv::Size patch_size) const {
        std::vector<WarpedMatPatch> patches;
        for(std::size_t i = 0; i < mats_.size(); i ++) {
            auto& fov = mats_[i];
            auto& stp = st_pts_[i];
            auto fov_r = r - stp.y;
            auto fov_c = c - stp.x;
            try {
                patches.emplace_back(
                    fov.at_real(fov_r, fov_c, patch_size)
                );
            } catch(...) {}
        }
        if(patches.empty()) {
            throw std::runtime_error(
                fmt::format("point out of boundary, real: ({}, {})", c, r)
            );
        }
        std::size_t min_cv_i = 0;
        for(std::size_t i = 1; i < patches.size(); i ++) {
            auto& ph = patches.at(i);
            if(ph.stat.cv < patches.at(min_cv_i).stat.cv) {
                min_cv_i = i;
            }
        }
        return patches.at(min_cv_i);
    }
private:
    std::vector<FOV>            mats_       ;
    std::vector<cv::Point2d>    st_pts_     ;

};

constexpr struct MakeMultiWarpedMat {
    template<class ImgPX>
    auto operator()(
        std::vector<
            WarpedMat<ImgPX, true>
        >&&                         mats,
        std::vector<cv::Point2d>&&  st_pts,
        cv::Point2d                 origin,
        double                      xd, 
        double                      yd
    ) const {
        return MultiWarpedMat<ImgPX, true>(
            mats, st_pts, origin,
            xd, yd
        );
    }
    template<class ImgPX>
    auto operator()(
        std::vector<
            WarpedMat<ImgPX, false>
        >&&                         mats,
        std::vector<cv::Point2d>&&  st_pts
    ) const {
        return MultiWarpedMat<ImgPX, false>(
            mats, st_pts
        );
    }

} make_multi_warped_mat;

}