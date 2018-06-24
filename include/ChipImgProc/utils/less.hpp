#pragma once
#include <ChipImgProc/utils/cv.h>
namespace chipimgproc{

constexpr struct PointLess {
    bool operator()(
        const cv::Point& p1, 
        const cv::Point& p2 
    ) const {
        if( p1.x == p2.x ) {
            return p1.y < p2.y;
        } else {
            return p1.x < p2.x;
        }
    }
} point_less;

}