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
 * @brief Margin algorithms toolkit parameters.
 * 
 * @tparam GLID The input tiled matrix grid line.
 */
template<
    class GLID = std::uint16_t
>
struct Param {
    /**
     * @brief The grid cell sampling/segmentation rate.
     * 
     */
    float                   seg_rate       ;

    /**
     * @brief Input tiled matrix.
     * 
     */
    TiledMat<GLID>* const   tiled_mat      ;

    /**
     * @brief Set to true will replace the tile data inside tiled matrix,
     *   otherwise the margin algorithm will only return the statistic result
     *   and not to change any data inside tiled matrix.
     * 
     */
    bool                    replace_tile   ;

    /**
     * @brief A debug image view functor 
     *   which showing the tiled matrix after margin.
     * 
     */
    std::function<
        void(const cv::Mat&)
    >                       v_result       ;
};


}
}