#pragma once
#include <cstdint>
#include <ChipImgProc/stat/mats.hpp>
namespace chipimgproc{ namespace margin{
/**
 * @brief A wrapper for margin algorithm result. 
 *   @copybrief chipimgproc::stat::Mats
 * 
 * @tparam FLOAT The float point type, 
 *   use to store the statistic data.
 */
template<class FLOAT = float>
struct Result {
    /**
     * @brief @copybrief chipimgproc::stat::Mats
     * 
     */
    stat::Mats<FLOAT> stat_mats;
};


}
}