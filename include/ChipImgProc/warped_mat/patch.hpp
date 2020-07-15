#pragma once
#include <ChipImgProc/stat/cell.hpp>

namespace chipimgproc::warped_mat {

struct Patch : public stat::Cell<double>{
    using Base = stat::Cell<double>;
    Patch(Base&& cell, cv::Mat pxs)
    : Base(std::move(cell))
    , patch(pxs)
    {}

    Patch(cv::Mat pxs)
    : Base(Base::make(pxs))
    , patch(pxs)
    {}
    cv::Mat patch;
};

}