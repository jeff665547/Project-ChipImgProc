#pragma once
#include <ChipImgProc/warped_mat/reg_mat_helper.hpp>
#include <ChipImgProc/warped_mat/patch.hpp>

namespace chipimgproc::multi_warped_mat {

template<class Derived, bool enable, class FOVAtResult>
struct MultiRegMatHelper 
: public warped_mat::RegMatHelper<Derived, enable, FOVAtResult>
{
    void init() {}
};

template<class Derived, class FOVAtResult>
struct MultiRegMatHelper<Derived, true, FOVAtResult> 
: public warped_mat::RegMatHelper<Derived, true, FOVAtResult>
{
    using Base = warped_mat::RegMatHelper<Derived, true, FOVAtResult>;
    using This = MultiRegMatHelper<Derived, true, FOVAtResult>;

    MultiRegMatHelper() = default;

    MultiRegMatHelper(
        cv::Point2d origin, 
        double xd, double yd,
        double x_max, double y_max
    )
    : Base(origin, xd, yd, x_max, y_max)
    {}
    template<class... Args>
    bool at_cell(warped_mat::Patch& res, std::int32_t r, std::int32_t c, Args&&... args) const {
        return derived()->at_each_fov(res, [&](FOVAtResult& tmp, std::size_t fov_i){
            auto& fov = derived()->mats_.at(fov_i);
            auto& stp = st_cl_pts_.at(fov_i);
            auto fov_r = r - stp.y;
            auto fov_c = c - stp.x;
            return fov.at_cell(tmp, fov_r, fov_c, FWD(args)...);
        });
    }
protected:
    void init() {
        for(std::size_t i = 0; i < derived()->mats_.size(); i ++) {
            auto& st_pt = derived()->st_pts_.at(i);
            auto& mat = derived()->mats_.at(i);
            auto bias = st_pt - derived()->origin();
            int st_cl_x = std::round(bias.x / derived()->xd());
            int st_cl_y = std::round(bias.y / derived()->yd());
            st_cl_pts_.emplace_back(
                st_cl_x, st_cl_y
            );
            Base::cl_x_n_ = std::max(mat.cols() + st_cl_x, Base::cl_x_n_);
            Base::cl_y_n_ = std::max(mat.rows() + st_cl_y, Base::cl_y_n_);
        }
    }
private:
    Derived* derived() {
        return static_cast<Derived*>(this);
    }
    const Derived* derived() const {
        return static_cast<const Derived*>(this);
    }

    std::vector<cv::Point>      st_cl_pts_  ;
};

}