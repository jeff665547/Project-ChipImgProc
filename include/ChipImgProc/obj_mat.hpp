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
        index_range_check(r, c);
        return obj_.at(index_(r, c));
    }
    decltype(auto) operator()(int r, int c) const {
        index_range_check(r, c);
        return obj_.at(index_(r, c));
    }
    const std::vector<T>& values() const {
        return obj_;
    }
private:
    void index_range_check(int r, int c) const {
        if(r < 0) throw std::out_of_range("ObjMat(): r < 0");
        if(c < 0) throw std::out_of_range("ObjMat(): c < 0");
        if(r >= index_.rows) throw std::out_of_range("ObjMat(): r >= index_.rows");
        if(c >= index_.cols) throw std::out_of_range("ObjMat(): c >= index_.cols");
    }
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