#pragma once
#include "reg_mat_helper.hpp"
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/obj_mat.hpp>
#include "patch.hpp"
namespace chipimgproc::warped_mat {

template<class Derived, bool enable, class Float>
struct StatRegMatHelper 
{
    template<class... Args>
    StatRegMatHelper(Args&&... args)
    {}
};

template<class Derived, class Float>
struct StatRegMatHelper<Derived, true, Float>
: public RegMatHelper<Derived, true>
{
    using Base      = RegMatHelper<Derived, true>;
    using CellMasks = ObjMat<cv::Mat, std::uint32_t>;

    StatRegMatHelper(
        stat::Mats<Float>&&     stat_mats,
        CellMasks        &&     cell_mask,
        cv::Point2d             origin, 
        double                  xd, 
        double                  yd, 
        double                  x_max, 
        double                  y_max
    )
    : Base              (origin, xd, yd, x_max, y_max)
    , stat_mats_        (std::move(stat_mats))
    , cell_mask_        (std::move(cell_mask))
    {}

    auto at_cell(std::int32_t r, std::int32_t c) const {
        if(r < 0 || r >= stat_mats_.rows()) 
            throw std::out_of_range("StatRegMatHelper: r < 0 || r >= rows");
        if(c < 0 || c >= stat_mats_.cols()) 
            throw std::out_of_range("StatRegMatHelper: c < 0 || c >= cols");
        auto stat = stat_mats_(r, c);
        auto mask = cell_mask_(r, c);
        auto pxs  = Base::at_cell(r, c, mask.size());
        // cv::Mat tmp  = pxs.patch.setTo(pxs.patch, mask);
        cv::Mat tmp = pxs.patch.mul(mask);
        return warped_mat::Patch(std::move(pxs), tmp);
        // return warped_mat::Patch(std::move(pxs), pxs.patch);
    }

    stat::Mats<Float>                   stat_mats_;
    CellMasks                           cell_mask_;
};

} // namespace chipimgproc::warped_mat
