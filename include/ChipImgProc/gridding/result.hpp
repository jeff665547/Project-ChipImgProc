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
     * @brief Grid table row numbers.
     * 
     */
    std::uint16_t               feature_rows;
    /**
     * @brief Grid table column numbers.
     * 
     */
    std::uint16_t               feature_cols;
    /**
     * @brief Grid table tiles.
     * 
     */
    std::vector<cv::Rect>       tiles;
    /**
     * @brief Grid line positions on x direction.
     * 
     */
    std::vector<double>         gl_x;
    /**
     * @brief Grid line positions on y direction.
     * 
     */
    std::vector<double>         gl_y; 
};

}}