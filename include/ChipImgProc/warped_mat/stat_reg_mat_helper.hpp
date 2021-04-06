#pragma once
#include "reg_mat_helper.hpp"
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/obj_mat.hpp>
#include "patch.hpp"
namespace chipimgproc::warped_mat {

template<class Derived, bool enable, class Float, class AtResult>
struct StatRegMatHelper 
{
    template<class... Args>
    StatRegMatHelper(Args&&... args)
    {}
};

template<class Derived, class Float, class AtResult>
struct StatRegMatHelper<Derived, true, Float, AtResult>
: public RegMatHelper<Derived, true, AtResult>
{
    using Base      = RegMatHelper<Derived, true, AtResult>;
    using CellMasks = ObjMat<cv::Mat, std::int32_t>;

    StatRegMatHelper() = default;
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

    bool at_cell(AtResult& res, std::int32_t r, std::int32_t c) const {
        if(r < 0 || r >= stat_mats_.rows()) 
            return false;
        if(c < 0 || c >= stat_mats_.cols()) 
            return false;
        auto stat = stat_mats_(r, c);
        auto mask = cell_mask_(r, c);
        auto pxs = Derived::make_at_result();
        if(!Base::at_cell(pxs, r, c, mask.size())) {
            return false;
        }
        auto img_p = pxs.img_p;
        auto real_p = pxs.real_p;
        cv::Mat tmp = pxs.patch.mul(mask);
        res = AtResult(
            std::move(pxs), std::move(tmp), 
            std::move(img_p), std::move(real_p)
        );
        return true;
    }

    stat::Mats<Float>                   stat_mats_;
    CellMasks                           cell_mask_;
};

} // namespace chipimgproc::warped_mat
