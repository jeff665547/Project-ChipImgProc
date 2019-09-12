/**
 * @file margin.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::Margin
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
 * @brief    This Margin class is a callable function object type.
 *           It provides a set of optional probe signal extraction methods for 
 *           summarizing the pixel intensities of each probe region of interest into 
 *           a collection of statistics such as mean, standard deviation, 
 *           coefficient of variation and the amount of pixels for calculation.
 * 
 * @tparam   FLOAT denotes a template parameter with a floating point variable type.
 *           This parameter determines the numerical accuracy of the summarization process.
 *           The precision of the floating point type reflects a trade off 
 *           between computational speed and numerical accuracy.
 * @tparam   GLID denotes a template parameter with the integer variable type, and
 *           generalizes the location of horizontal and vertical grid lines in pixels.
 */
template<
    class FLOAT = float, 
    class GLID  = std::uint16_t
>
struct Margin {
    /**
     * @brief    This is a function call operator. It summarize the pixel intensities of
     *           each probe region of interest into a collection of statistics.
     *
     * @param    method refers to the string token denoting the option of extraction methods.
     *           Currently, the following string tokens and methods are available.
     *
     *           * auto_min_cv
     *             
     *             This method takes the center 66% of the area from the given tile, and then
     *             searches a smaller window from the shrinkage area based on minimum CV criterion.
     *             The window size is specified by the parameter @a param.seg_rate, 
     *             which refers to the relative size of the shrinkage area.
     *              
     *           * mid_seg
     *
     *             This method takes the center region of the area from the given tile object to
     *             summarize the probe signal. The size of center region is specified by the 
     *　　　　　　 parameter @a param.seg_rate, which refers to the relative size of the given tile area.
     * 
     *           * percentile
     *
     *             This method takes the center 60% of the area from the given tile, and then
     *             takes the percentile of pixel intensities from the shrinkage area to summarize the probe signal.
     *             The setting of percentile is specified by the parameter @a param.seg_rate, 
     *             which refers to a ratio value between 0 and 1.
     * 
     *           * max
     *     
     *             This method takes the center 60% of the area from the given tile, and then
     *             takes the maximum of pixel intensities from the shrinkage area to summarize the probe signal.
     * 
     *           * only_stat
     *     
     *             This method takes all of pixel intensities within the area 
     *             specified by the given tile object for summarization.
     *             The parameter @a param.seg_rate is unused.
     * 
     * @param    param the a parameter pack which contains the segmentation rate (between 0 and 1), 
     *           tiled matrix, and a boolean flag to indicate whether each tile of image content 
     *           should be replaced by results of calculation. 
     *           Please see the class chipimgproc::margin::Param for details
     *
     * @return   a matrix of statistics with chipimgproc::margin::Result<FLOAT> class type,
     *           which includes the means, standard deviations, coefficients of variation, 
     *           and the amount of pixels for summarization.
     *           Please see the class chipimgproc::margin::Result for details
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