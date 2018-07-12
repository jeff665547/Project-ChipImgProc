#pragma once
#include <ChipImgProc/utils.h>
#include <iostream>
namespace chipimgproc{ namespace marker{ namespace detection{
struct MKRegion : public cv::Rect {
    int x_i;
    int y_i;
    void info(std::ostream& out) {
        out << "(" << this->x << "," << this->y << ")"
            << "[" << this->width << "," << this->height << "]"
            << "(" << x_i << "," << y_i << ")"
            << std::endl
        ;
    }
};
}}}