#pragma once
#include <cstdint>
#include <ChipImgProc/stat/mats.hpp>
namespace chipimgproc{ namespace margin{
template<class FLOAT = float>
struct Result {
    stat::Mats<FLOAT> stat_mat_;
};


}
}