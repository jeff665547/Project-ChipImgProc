#pragma once
#include "warped_mat/patch.hpp"
#include "warped_mat/reg_mat_helper.hpp"
#include <stdexcept>
#include "multi_warped_mat/mincv_reducer.hpp"
namespace chipimgproc {

template<
    class FOV, 
    bool is_reg_mat = true,
    template<class _FOV> class ReducerTpl = multi_warped_mat::MinCVReducer
>
struct MultiWarpedMat 
: public warped_mat::RegMatHelper<
    MultiWarpedMat<FOV, is_reg_mat>,
    is_reg_mat
>
{
    using Reducer = ReducerTpl<FOV>;
    using Base = warped_mat::RegMatHelper<
        MultiWarpedMat<FOV, is_reg_mat>,
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

        for(auto&& st_pt : st_pts_) {
            auto bias = st_pt - Base::origin();
            int st_cl_x = std::round(bias.x / Base::xd());
            int st_cl_y = std::round(bias.y / Base::yd());
            st_cl_pts_.emplace_back(
                st_cl_x, st_cl_y
            );
        }
    }
private:
    template<class Func>
    auto at_each_fov(Func&& access) const {
        using CellType = decltype(access(std::size_t()));
        std::vector<CellType> patches;
        for(std::size_t i = 0; i < mats_.size(); i ++) {
            try {
                patches.emplace_back(access(i));
            } catch(...) {}
        }
        if(patches.empty()) {
            throw std::out_of_range("point out of boundary");
        }
        return reducer_(patches);
    }
public:
    auto at_real(double r, double c, cv::Size patch_size) const {
        try {
            return at_each_fov([&](std::size_t fov_i){
                auto& fov = mats_.at(fov_i);
                auto& stp = st_pts_.at(fov_i);
                auto fov_r = r - stp.y;
                auto fov_c = c - stp.x;
                return fov.at_real(fov_r, fov_c, patch_size);
            });
        } catch(const std::out_of_range& e) {
            throw std::out_of_range(fmt::format("at_real({},{},[{},{}])", 
                r, c, patch_size.width, patch_size.height
            ));
        }
    }
    template<class... Args>
    auto at_cell(std::int32_t r, std::int32_t c, Args&&... args) const {
        try {
            return at_each_fov([&](std::size_t fov_i){
                auto& fov = mats_.at(fov_i);
                auto& stp = st_cl_pts_.at(fov_i);
                auto fov_r = r - stp.y;
                auto fov_c = c - stp.x;
                return fov.at_cell(fov_r, fov_c, FWD(args)...);
            });
        } catch(const std::out_of_range& e) {
            throw std::out_of_range(fmt::format("at_cell({},{},...)", 
                r, c
            ));
        }
    }
private:
    std::vector<FOV>            mats_       ;
    std::vector<cv::Point2d>    st_pts_     ;
    std::vector<cv::Point>      st_cl_pts_  ;
    Reducer                     reducer_    ;

};

constexpr struct MakeMultiWarpedMat {
    template<class FOV>
    auto operator()(
        std::vector<FOV>&&          mats,
        std::vector<cv::Point2d>&&  st_pts,
        cv::Point2d                 origin,
        double                      xd, 
        double                      yd
    ) const {
        return MultiWarpedMat<FOV>(
            std::move(mats), 
            std::move(st_pts), 
            origin,
            xd, yd
        );
    }
    template<class FOV>
    auto operator()(
        std::vector<FOV>&&          mats,
        std::vector<cv::Point2d>&&  st_pts
    ) const {
        return MultiWarpedMat<FOV, false>(
            std::move(mats), std::move(st_pts)
        );
    }
} make_multi_warped_mat;

}