#pragma once
#include <ChipImgProc/utils/cv.h>
#include <ChipImgProc/utils/less.hpp>
namespace chipimgproc { 
/* rows cols */
template<class M>
auto cols(const M& m) {
    return m.cols();
}

template<class M>
auto rows(const M& m) {
    return m.rows();
}

template<class T>
auto cols(const cv::Mat_<T>& m ) {
    return m.cols;
}
template<class T>
auto rows(const cv::Mat_<T>& m ) {
    return m.rows;
}

int cols(const cv::Mat& m);
int rows(const cv::Mat& m);


/* roi */
template<class T>
auto get_roi( T& m, const cv::Rect& rect) {
    return m.get_roi(rect);
}
template<class T>
auto get_roi( cv::Mat_<T>& m, const cv::Rect& rect) {
    return m(rect);
}
cv::Mat get_roi( cv::Mat& m, const cv::Rect& rect );

}