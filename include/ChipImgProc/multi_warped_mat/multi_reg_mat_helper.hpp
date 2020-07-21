#pragma once
#include <ChipImgProc/warped_mat/reg_mat_helper.hpp>

namespace chipimgproc::multi_warped_mat {

template<class Derived, bool enable>
struct MultiRegMatHelper 
: public warped_mat::RegMatHelper<Derived, enable>
{
    void init() {}
};

template<class Derived>
struct MultiRegMatHelper<Derived, true> 
: public warped_mat::RegMatHelper<Derived, true>
{
    using Base = warped_mat::RegMatHelper<Derived, true>;
    using This = MultiRegMatHelper<Derived, true>;

    MultiRegMatHelper(
        cv::Point2d origin, 
        double xd, double yd,
        double x_max, double y_max
    )
    : Base(origin, xd, yd, x_max, y_max)
    {}
    template<class... Args>
    auto at_cell(std::int32_t r, std::int32_t c, Args&&... args) const {
        try {
            return derived()->at_each_fov([&](std::size_t fov_i){
                auto& fov = derived()->mats_.at(fov_i);
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