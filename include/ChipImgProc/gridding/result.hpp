/**
 * @file result.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::gridding::Result
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{ namespace gridding{
/**
 * @brief The general gridding result data pack.
 * 
 */
struct Result {
    /**
     * @brief Grid table row and column numbers.
     * 
     */
    std::uint16_t               feature_rows, feature_cols  ;
    /**
     * @brief Grid table tiles.
     * 
     */
    std::vector<cv::Rect>       tiles                       ;
    /**
     * @brief Grid line positions on both x and y direction.
     * 
     */
    std::vector<double>         gl_x, gl_y                  ;
};

}}