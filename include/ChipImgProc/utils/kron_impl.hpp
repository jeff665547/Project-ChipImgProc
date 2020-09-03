#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc {

cv::Mat kron(cv::Mat a, cv::Mat b) {
    if(a.channels() != 1) throw std::runtime_error("first matrix channel number must be 1");
    cv::Mat res(a.rows * b.rows, a.cols * b.cols, b.type());
    for(int i = 0; i < a.rows; i ++) {
        for(int j = 0; j < a.cols; j ++) {
            typed_mat(a, [&](auto& mat){
                res(cv::Rect(j * b.cols, i * b.rows, b.cols, b.rows)) = mat(i, j) * b;
            });
        }
    }
    return res;
}
    
} // namespace chipimgproc
