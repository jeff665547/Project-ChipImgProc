#pragma once
#include <opencv2/opencv.hpp>
namespace chipimgproc::utils {

struct PosCompByScore {
    PosCompByScore(const cv::Mat_<float>& score)
    : score_(score)
    {}
    bool operator()(
        const cv::Point& p0, 
        const cv::Point& p1
    ) const {
        auto& s0 = score_(p0.y, p0.x);
        auto& s1 = score_(p1.y, p1.x);
        if( s0 == s1 ) {
            if( p0.x == p1.x ) {
                return p0.y < p1.y;
            } else return p0.x < p1.x;
        } else return s0 < s1;
        
    }
private:
    const cv::Mat_<float>& score_;
};

}