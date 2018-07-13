#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{ namespace gridding{

struct Result {
    std::uint16_t               feature_rows, feature_cols  ;
    std::vector<cv::Rect>       tiles                       ;
    std::vector<std::uint32_t>  gl_x, gl_y                  ;
};

}}