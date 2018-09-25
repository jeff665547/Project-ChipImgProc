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
    const auto& gl_x() const  { return gl_x_; }
    auto&       gl_x()        { return gl_x_; }
    const auto& gl_y() const  { return gl_y_; }
    auto&       gl_y()        { return gl_y_; }
    bool        empty()       { return img_.empty(); }
    bool        empty() const { return img_.empty(); }

    GridRawImg  get_roi(const cv::Rect& r) const {
        cv::Rect raw_rect(
            gl_x_.at(r.x), gl_y_.at(r.y),
            gl_x_.at(r.x + r.width ) - gl_x_.at(r.x),
            gl_y_.at(r.y + r.height) - gl_y_.at(r.y)
        );
        auto img = img_(raw_rect);
        std::vector<GLID> gl_x(
            gl_x_.begin() + r.x, 
            gl_x_.begin() + r.x + r.width + 1
        );
        std::vector<GLID> gl_y(
            gl_y_.begin() + r.y, 
            gl_y_.begin() + r.y + r.height + 1
        );
        return GridRawImg(img, gl_x, gl_y);
    }

    GridRawImg clone() const {
        return GridRawImg(
            img_.clone(),
            gl_x_, gl_y_
        );
    }

private:
    cv::Mat img_;
    std::vector<GLID> gl_x_;
    std::vector<GLID> gl_y_;
};



}