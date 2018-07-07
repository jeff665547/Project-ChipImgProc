#pragma once
#include <cstdint>
#include <ChipImgProc/margin/auto_min_cv.hpp>
#include <ChipImgProc/margin/param.hpp>
#include <ChipImgProc/margin/result.hpp>
namespace chipimgproc{
template<class FLOAT = float>
struct Margin {

// algorithm routing
margin::Result<FLOAT> operator()(const std::string& method, const margin::Param& param) {
    if(method == "auto_min_cv") {
        margin::AutoMinCV<FLOAT> auto_min_cv;
        margin::Result<FLOAT> res;
        auto tmp = auto_min_cv(*param.tiled_mat, param.windows_width, param.windows_height);
        res.stat_mat_ = tmp;
        return res;
    } else {
        throw std::runtime_error("unknown algorithm name: " + method);
    }
}

};

}