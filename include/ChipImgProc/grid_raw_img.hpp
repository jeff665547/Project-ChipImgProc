#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range/irange.hpp>
namespace chipimgproc{

template<class GLID>
struct GridRawImg {
    GridRawImg() = default;
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
    const auto& gl_x() const { return gl_x_; }
    auto&       gl_x()       { return gl_x_; }
    const auto& gl_y() const { return gl_y_; }
    auto&       gl_y()       { return gl_y_; }
    bool empty()        { return img_.empty(); }
    bool empty() const  { return img_.empty(); }
private:
    cv::Mat img_;
    std::vector<GLID> gl_x_;
    std::vector<GLID> gl_y_;
};

}