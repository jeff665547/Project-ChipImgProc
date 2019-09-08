/**
 * @file grid_raw_img.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::GridRawImg
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range.hpp>
namespace chipimgproc{

template<class GLID = std::uint16_t>
struct GridRawImg {
    GridRawImg() = default;

    template<class MAT, class GLX, class GLY>
    GridRawImg(
        MAT&& img, 
        const GLX& glx,
        const GLY& gly
    )
    : img_  (FWD(img))
    , gl_x_ ()
    , gl_y_ ()
    {
        gl_x_.reserve(glx.size());
        gl_y_.reserve(gly.size());
        for(auto&& x : glx) {
            gl_x_.push_back((GLID)x);
        }
        for(auto&& y : gly) {
            gl_y_.push_back((GLID)y);
        }
    }
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
    auto        rows()        { return img_.rows; } 
    auto        cols()        { return img_.cols; } 
    auto        rows()  const { return img_.rows; } 
    auto        cols()  const { return img_.cols; } 

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
        auto front = gl_x.front();
        for(auto& l : gl_x) {
            l -= front;
        }
        front = gl_y.front();
        for(auto& l : gl_y) {
            l -= front;
        }
        return GridRawImg(img, gl_x, gl_y);
    }
    auto clean_border() const {
        cv::Rect roi(
            0, 0, 
            gl_x_.size() - 1, gl_y_.size() - 1
        );
        return get_roi(roi);
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