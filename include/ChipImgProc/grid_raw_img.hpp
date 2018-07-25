#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range/irange.hpp>
namespace chipimgproc{

template<class GLID>
struct GridRawImg {
    GridRawImg(
        const cv::Mat& img, 
        const std::vector<GLID> glx,
        const std::vector<GLID> gly
    )
    : img_  (img)
    , gl_x_ (glx)
    , gl_y_ (gly)
    {}
    auto& mat() {
        return img_;
    }
    const auto& mat() const {
        return img_;
    }
private:
    cv::Mat img_;
    std::vector<GLID> gl_x_;
    std::vector<GLID> gl_y_;
};

}