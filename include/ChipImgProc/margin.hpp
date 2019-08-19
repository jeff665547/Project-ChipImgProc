#pragma once
#include <cstdint>
#include <ChipImgProc/margin/percentile.hpp>
#include <ChipImgProc/margin/auto_min_cv.hpp>
#include <ChipImgProc/margin/mid_seg.hpp>
#include <ChipImgProc/margin/only_stat.hpp>
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
    ) const {
        margin::Result<FLOAT> res;
        if(method == "auto_min_cv") {
            margin::AutoMinCV<FLOAT> auto_min_cv;
            auto tmp = auto_min_cv(
                *param.tiled_mat, 
                param.seg_rate,
                param.replace_tile,
                param.v_result
            );
            res.stat_mats = tmp;
        } else if(method == "mid_seg") {
            margin::MidSeg<FLOAT> mid_seg;
            auto tmp = mid_seg(
                *param.tiled_mat, 
                param.seg_rate,
                param.replace_tile,
                param.v_result
            );
            res.stat_mats = tmp;
        } else if(method == "percentile") {
            margin::Percentile<FLOAT> func;
            auto tmp = func(
                *param.tiled_mat, 
                param.seg_rate,
                param.replace_tile,
                param.v_result
            );
            res.stat_mats = tmp;
        } else if(method == "max") {
            margin::Max<FLOAT> func;
            auto tmp = func(
                *param.tiled_mat, 
                param.replace_tile,
                param.v_result
            );
            res.stat_mats = tmp;
        } else if( method == "only_stat") {
            margin::OnlyStat<FLOAT> only_stat;
            auto tmp = only_stat(
                *param.tiled_mat,
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