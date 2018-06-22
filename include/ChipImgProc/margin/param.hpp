#pragma once
#include <cstdint>
#include <ChipImgProc/tiled_mat.hpp>
namespace chipimgproc{ namespace margin{

struct Param {
    std::int32_t        windows_width  ;
    std::int32_t        windows_height ;
    TiledMat<>* const   tiled_mat      ;
};


}
}