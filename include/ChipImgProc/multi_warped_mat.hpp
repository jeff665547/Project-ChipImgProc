#pragma once
#include "warped_mat/patch.hpp"
#include "warped_mat/reg_mat_helper.hpp"
#include <stdexcept>
#include "multi_warped_mat/mincv_reducer.hpp"
#include "multi_warped_mat/multi_reg_mat_helper.hpp"

namespace chipimgproc {

template<
    class FOV, 
    bool is_reg_mat = true,
    template<class _FOV> class ReducerTpl = multi_warped_mat::MinCVReducer
>
struct MultiWarpedMat 
: public multi_warped_mat::MultiRegMatHelper<
    MultiWarpedMat<FOV, is_reg_mat>,
    is_reg_mat,
    typename FOV::AtResult
>
{
    using Reducer = ReducerTpl<FOV>;
    using Base = multi_warped_mat::MultiRegMatHelper<
        MultiWarpedMat<FOV, is_reg_mat>,
        is_reg_mat,
        typename FOV::AtResult
    >;
    friend Base;
    using FOVAtResult = typename FOV::AtResult; 
    using AtResult = warped_mat::Patch;

    MultiWarpedMat() = default;

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
        Base::init();
    }
protected:
    template<class Func>
    bool at_each_fov(AtResult& res, Func&& access) const {
        // using CellType = decltype(access(std::size_t()));
        std::vector<FOVAtResult> patches;
        FOVAtResult tmp;
        for(std::size_t i = 0; i < mats_.size(); i ++) {
            if(!access(tmp, i)) continue;
            patches.emplace_back(tmp);
        }
        if(patches.empty()) {
            return false;
        }
        res = reducer_(patches);
        return true;
    }
public:
    bool at_real(AtResult& res, double r, double c, cv::Size patch_size) const {
        return at_each_fov(res, [&](FOVAtResult& tmp, std::size_t fov_i){
            auto& fov = mats_.at(fov_i);
            auto& stp = st_pts_.at(fov_i);
            auto fov_r = r - stp.y;
            auto fov_c = c - stp.x;
            return fov.at_real(tmp, fov_r, fov_c, patch_size);
        });
    }
    std::vector<cv::Mat> warp_mats() const {
        std::vector<cv::Mat> res;
        for(auto&& m : mats_) {
            res.push_back(m.warp_mat().clone());
        }
        return res;
    }
    static AtResult make_at_result() {
        return {};
    }
private:
    std::vector<FOV>            mats_       ;
    std::vector<cv::Point2d>    st_pts_     ;
    Reducer                     reducer_    ;

};

constexpr struct MakeMultiWarpedMat {
    template<class FOV>
    auto operator()(
        std::vector<FOV>&&          mats,
        std::vector<cv::Point2d>&&  st_pts,
        cv::Point2d                 origin,
        double                      xd, 
        double                      yd,
        double                      max_x,
        double                      max_y 
    ) const {
        return MultiWarpedMat<FOV>(
            std::move(mats), 
            std::move(st_pts), 
            origin,
            xd, yd,
            max_x, max_y
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