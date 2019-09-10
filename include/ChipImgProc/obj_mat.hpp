#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc {

template<class T, class ID = std::uint16_t>
struct ObjMat {
    ObjMat() = default;
    ObjMat(int r, int c)
    : obj_(r*c)
    , index_(build_index(r, c))
    {
    }
    void resize(int r, int c) {
        obj_.resize(r * c);
        index_ = build_index(r, c);
    }
    decltype(auto) operator()(int r, int c) {
        return obj_.at(index_(r, c));
    }
    decltype(auto) operator()(int r, int c) const {
        return obj_.at(index_(r, c));
    }
    const std::vector<T>& values() const {
        return obj_;
    }
private:
    static cv::Mat_<ID> build_index(int r, int c) {
        cv::Mat_<ID> index(r, c);
        ID n = 0;
        for(int i = 0; i < r; i ++ ) {
            for(int j = 0; j < c; j ++ ) {
                index(i, j) = n ++; 
            }
        }
        return index;
    }
    std::vector<T> obj_;
    cv::Mat_<ID> index_;
};

}