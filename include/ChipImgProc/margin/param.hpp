#pragma once
#include <cstdint>
#include <ChipImgProc/tiled_mat.hpp>
namespace chipimgproc{ namespace margin{

template<
    class GLID = std::uint16_t
>
struct Param {
    float                   seg_rate       ;
    TiledMat<GLID>* const   tiled_mat      ;
    bool                    replace_tile   ;
    std::function<
        void(const cv::Mat&)
    >                       v_result       ;
};


}
}