#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc::wraped_mat {

template<class Derived, bool enable>
struct RegMatHelper {};

template<class Derived>
struct RegMatHelper<Derived, false> {};

template<class Derived>
struct RegMatHelper<Derived, true> {
    using This = RegMatHelper<Derived, true>;
    
    RegMatHelper(cv::Point2d origin, double xd, double yd)
    : origin_   (origin)
    , xd_       (xd)
    , yd_       (yd)
    {}

    auto at_cell(
        std::int32_t r, 
        std::int32_t c, 
        cv::Size patch_size = cv::Size(5, 5)
    ) const {
        auto cent_c = (c * xd_) + origin_.x + (xd_ / 2);
        auto cent_r = (r * yd_) + origin_.y + (yd_ / 2);
        return derived()->at_real(cent_r, cent_c, patch_size);
    }

private:
    Derived* derived() {
        return static_cast<Derived*>(this);
    }
    const Derived* derived() const {
        return static_cast<const Derived*>(this);
    }
    cv::Point2d origin_ ; // real domain, cell left top
    double      xd_     ;
    double      yd_     ;
};

}