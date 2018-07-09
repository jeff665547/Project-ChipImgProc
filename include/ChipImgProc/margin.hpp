#pragma once
#include <cstdint>
#include <ChipImgProc/margin/auto_min_cv.hpp>
#include <ChipImgProc/margin/mid_seg.hpp>
#include <ChipImgProc/margin/param.hpp>
#include <ChipImgProc/margin/result.hpp>
namespace chipimgproc{

template<
    class FLOAT = float, 
    class GLID  = std::uint16_t
>
struct Margin {

    // algorithm routing
    margin::Result<FLOAT> operator()(
        const std::string&          method, 
        const margin::Param<GLID>&  param
    ) {
        margin::Result<FLOAT> res;
        if(method == "auto_min_cv") {
            margin::AutoMinCV<FLOAT> auto_min_cv;
            auto tmp = auto_min_cv(
                *param.tiled_mat, 
                param.seg_rate,
                param.v_result
            );
            res.stat_mats = tmp;
        } else if(method == "mid_seg") {
            margin::MidSeg<FLOAT> mid_seg;
            auto tmp = mid_seg(
                *param.tiled_mat, 
                param.seg_rate,
                param.v_result
            );
            res.stat_mats = tmp;
        } else {
            throw std::runtime_error("unknown algorithm name: " + method);
        }
        return res;
    }

};

}