/**
 * @file mats.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::stat::Mats
 * 
 */

#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
namespace chipimgproc { namespace stat{

/**
 * @brief A statistic matrix set contains the mean, 
 *   standard deviation coefficient of variation and pixel numbers for each grid cell.
 * 
 * @tparam FLOAT The float point type, use to store the statistic data.
 */
template<class FLOAT = float>
struct Mats
{
    /**
     * @brief The float point type, use to store the statistic data.
     * 
     */
    using FloatType = FLOAT;

    /**
     * @brief Create a empty Mats.
     * 
     */
    Mats() = default;

    /**
     * @brief Create a empty Mats with row and column numbers.
     * 
     * @param rows Matrix row.
     * @param cols Matrix column.
     */
    Mats(int rows, int cols)
    : mean  (rows, cols)
    , stddev(rows, cols)
    , cv    (rows, cols)
    , num   (rows, cols)
    {}

    /**
     * @brief Select a intrested region and reset to the Mats.
     * 
     * @param r A region of interested.
     */
    void roi(const cv::Rect& r) {
        mean   = mean   (r);
        stddev = stddev (r);
        cv     = cv     (r);
        num    = num    (r);
    }

    /**
     * @brief A matrix of mean value.
     * 
     */
    cv::Mat_<FLOAT>         mean    ;

    /**
     * @brief A matrix of standard deviation.
     * 
     */
    cv::Mat_<FLOAT>         stddev  ;

    /**
     * @brief A matrix of coefficient of variation.
     * 
     */
    cv::Mat_<FLOAT>         cv      ;

    /**
     * @brief A matrix of cell pixel numbers.
     * 
     */
    cv::Mat_<std::uint32_t> num     ;
};

}
}