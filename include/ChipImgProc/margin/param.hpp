/**
 * @file param.hpp
 * @author Chia-Hua Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::margin::Param
 * 
 */
#pragma once
#include <cstdint>
#include <ChipImgProc/tiled_mat.hpp>
namespace chipimgproc{ namespace margin{
/**
 * @brief    This Param class is a pack of input parameters
 *           for the class chipimgproc::Margin<FLOAT,GLID>
 * 
 * @tparam   GLID denotes a template parameter with the integer variable type, and
 *           generalizes the location of horizontal and vertical grid lines in pixels.
 */
template<
    class GLID = std::uint16_t
>
struct Param {
    /**
     * @brief    typically, a floating point value between 0 and 1 inclusive.
     *           the following shows the definitions of this parameter
     *           in all configurations of chipimgproc::Margin algorithm.
     * 
     *           * In @b auto_min_cv method, this parameter refers to a 
     *             relative window size against the 66% of the tile size.
     * 
     *           * In @b mid_seg method, this parameter refers to a 
     *             relative window size against the whole tile size.
     * 
     *           * In @b percentile method, this parameter refers to the 
     *             percentage in the interval [0, 1], where the 1 means 100%.
     * 
     *           * In @b max and @b only_stat method, this parameter is unused.
     */
    float                   seg_rate       ;

    /**
     * @brief    a pointer to the TiledMat<GLID> structure, which stores the data
     *           related to the gridding results and parameters.
     * 
     */
    TiledMat<GLID>* const   tiled_mat      ;

    /**
     * @brief    a boolean flag. If @a replace_tile is True, then the pixel values inside 
     *           each tile will be replaced by the statistic results, that is,
     *           the input image will be changed. Otherwise, 
     *           it preserves the original pixel values. 
     */
    bool                    replace_tile   ;

    /**
     * @brief    a function wrapper. This wrapper will be invoked to render the content 
     *           of the TileMat object onto the debug image after applying the method.
     */
    std::function<
        void(const cv::Mat&)
    >                       v_result       ;
};


}
}