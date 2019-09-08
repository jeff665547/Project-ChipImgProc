/**
 * @file margin.hpp
 * @author Chia-Hua Chang(johnidfet@centrilliontech.com.tw)
 * @brief @copybrief MultiTiledMat::Margin
 * 
 */
#pragma once
#include <cstdint>
#include <ChipImgProc/margin/max.hpp>
#include <ChipImgProc/margin/percentile.hpp>
#include <ChipImgProc/margin/auto_min_cv.hpp>
#include <ChipImgProc/margin/mid_seg.hpp>
#include <ChipImgProc/margin/only_stat.hpp>
#include <ChipImgProc/margin/param.hpp>
#include <ChipImgProc/margin/result.hpp>

namespace chipimgproc{

/**
 * @brief The margin functor type, doing statistic 
 *        and cell ROI to the gridding result.
 * 
 * @tparam FLOAT The float point type used in this data structure, 
 *   and is use to trade off the performance and numerical accuracy.
 * @tparam GLID The grid line type used by input tiled matrix.
 */
template<
    class FLOAT = float, 
    class GLID  = std::uint16_t
>
struct Margin {
    /**
     * @brief Call operator, call the margin method by given name.
     *   Doing statistic and cell ROI to the gridding result.
     * 
     * @param method Margin method name, 
     *   there are serval choice and description:
     * 
     *   * auto_min_cv
     * 
     *     Autometic search the minimum cv region in the grid cell.
     *     The margin process will first shrink the cell space 
     *     inward by 34% on both direction,
     *     and search the minimum cv region 
     *     with @a param.seg_rate size in the space.
     * 
     *   * mid_seg
     * 
     *     Only do the middle segmentation by given @a param.seg_rate.
     * 
     *   * percentile
     * 
     *     The margin process will first shrink the cell space 
     *     inward by 20% on both direction,
     *     and sampling middle @a param.seg_rate part 
     *     of current space pixel distribution.
     * 
     *   * max
     *     
     *     The margin process will first shrink the cell space 
     *     inward by 20% on both direction,
     *     and select the max pixel value in current space.
     * 
     *   * only_stat
     *     
     *     Do not do any sampling.
     * 
     *  The method auto_min_cv, mid_seg and only_stat 
     *  will do the CV computing after the cell segmentation, 
     *  while the percentile and max method will not do the 
     *  CV computing after the cell segmentation. 
     * 
     * @param param The margin parameter pack, 
     *              include segmentation rate, tiled matrix 
     *              and a flag for determine
     *              if the margin result tiles should replace 
     *              the origin tiled matrix. 
     *              See the class margin::Param document for details.
     * 
     * @return margin::Result<FLOAT> The result statistic data matrix,
     *              include mean, standard deviation, coefficient of variation, pixel numbers.
     *   
     */
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