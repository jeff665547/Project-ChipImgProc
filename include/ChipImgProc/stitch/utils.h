#pragma once
#include <ChipImgProc/utils.h>
#include <iostream>
#include <vector>
namespace chipimgproc{ namespace stitch{

cv::Rect get_full_w_h( 
    const std::vector<cv::Mat>& imgs, 
    const std::vector<cv::Point_<int>>& st_ps
);
cv::Mat add(
    const std::vector<cv::Mat>& imgs, 
    const std::vector<cv::Point_<int>>& st_ps
);
}}