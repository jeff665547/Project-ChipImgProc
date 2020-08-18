#pragma once
#include <ChipImgProc/stat/cell.hpp>

namespace chipimgproc::warped_mat {
struct RawPatch {
    cv::Mat patch;
    cv::Point2d img_p;
    cv::Point2d real_p;
};
struct Patch 
: public RawPatch
, public stat::Cell<double>{
    using sCell = stat::Cell<double>;
    Patch(
        stat::Cell<double>&& cell, 
        cv::Mat           && pxs, 
        cv::Point2d       && img_p, 
        cv::Point2d       && real_p
    )
    : RawPatch {pxs, std::move(img_p), std::move(real_p)}
    , sCell(std::move(cell))
    {}

    Patch(
        cv::Mat              pxs, 
        cv::Point2d       && img_p, 
        cv::Point2d       && real_p
    )
    : RawPatch {pxs, std::move(img_p), std::move(real_p)}
    , sCell(sCell::make(pxs))
    {}

    Patch(
        RawPatch&& rpatch
    )
    : RawPatch(std::move(rpatch))
    , sCell(sCell::make(this->patch))
    {}
};

}