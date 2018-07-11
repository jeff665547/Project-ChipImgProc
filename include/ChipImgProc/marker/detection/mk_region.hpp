#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{ namespace marker{ namespace detection{
struct MKRegion : public cv::Rect {
    int x_i;
    int y_i;
};
}}}