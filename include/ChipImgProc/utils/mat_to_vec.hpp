#pragma once
#include <vector>
#include <ChipImgProc/utils.h>

namespace chipimgproc::utils {

template<class T>
std::vector<T> mat__to_vec(cv::Mat_<T> mat) {
    std::vector<T> res(mat.begin(), mat.end());
    return res;
}
template<class T>
std::vector<T> mat_to_vec(cv::Mat mat) {
    std::vector<T> res(mat.begin<T>(), mat.end<T>());
    return res;
}

}