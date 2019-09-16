/**
 * @file ChipImgProc/margin/result.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw) 
 * @brief @copybrief chipimgproc::margin::Result
 */
#pragma once
#include <cstdint>
#include <ChipImgProc/stat/mats.hpp>
namespace chipimgproc{ namespace margin{
/**
 * @brief    This template structure is a wrapper containing 
 *           the results of the chipimgproc::Margin callable object.
 *           see chipimgproc::stat::Mats for details.
 * 
 * @tparam   FLOAT denotes the floating point variable type.
 *           This template parameter generalizes the 
 *           storing type of the statistic data.
 */
template<class FLOAT = float>
struct Result {
    /**
     * @brief @copybrief chipimgproc::stat::Mats
     */
    stat::Mats<FLOAT> stat_mats;
};


}
}