#pragma once
#include "reg_mat_helper.hpp"
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/obj_mat.hpp>
#include "patch.hpp"
namespace chipimgproc::warped_mat {

template<class Derived, bool enable>
struct StatRegMatHelper 
{};

template<class Derived>
struct StatRegMatHelper<Derived, true>
: protected RegMatHelper<Derived, true>
{
    using Base      = RegMatHelper<Derived, true>;
    using CellMasks = ObjMat<cv::Mat, std::uint32_t>;

    StatRegMatHelper(
        stat::Mats<double>&&    stat_mats,
        CellMasks         &&    cell_mask,
        cv::Point2d             origin, 
        double                  xd, 
        double                  yd
    )
    : Base              (origin, xd, yd)
    , stat_mats_        (std::move(stat_mats))
    , cell_mask_        (std::move(cell_mask))
    {}

    auto at_cell(std::int32_t r, std::int32_t c) const {
        auto stat = stat_mats_(r, c);
        auto mask = cell_mask_(r, c);
        auto pxs  = Base::at_cell(r, c, 0, mask.size());
        cv::Mat tmp  = pxs & mask;
        return warped_mat::Patch(std::move(pxs), tmp);
    }

    stat::Mats<double>                  stat_mats_;
    CellMasks                           cell_mask_;
};

} // namespace chipimgproc::warped_mat
