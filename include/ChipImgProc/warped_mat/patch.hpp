#pragma once
#include <ChipImgProc/stat/cell.hpp>

namespace chipimgproc::warped_mat {
// (*) 
struct CellPos {
    cv::Point2d img_p;
    cv::Point2d real_p;
};
struct RawPatch
: public CellPos {
    RawPatch() = default;
    RawPatch(
        cv::Mat           patch,
        cv::Point2d       img_p,
        cv::Point2d       real_p
    )
    : CellPos {std::move(img_p), std::move(real_p)}
    , patch   (std::move(patch))
    {}

    cv::Mat patch;
};
// struct RawPatch {
//     cv::Mat patch;
//     cv::Point2d img_p;
//     cv::Point2d real_p;
// };
struct Patch 
: public RawPatch
, public stat::Cell<float>{
    using sCell = stat::Cell<float>;
    Patch() = default;
    Patch(
        stat::Cell<float>&& cell, 
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
        stat::Cell<float> && cell, 
        RawPatch          && rpatch
    )
    : RawPatch(std::move(rpatch))
    , sCell(std::move(cell))
    {}

    Patch(
        RawPatch&& rpatch
    )
    : RawPatch(std::move(rpatch))
    , sCell(sCell::make(this->patch))
    {}
};

}