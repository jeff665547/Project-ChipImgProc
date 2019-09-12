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
 * @brief    This Mats class contains a collection of matrices 
 *           for storing the mean values, standard deviations, 
 *           coefficients of variation, and the amount of pixels 
 *           for summarization for each grid cell.
 * 
 * @tparam   FLOAT denotes the floating point vriable type.
 *           This template parameter generalizes the 
 *           storing type of the statistic data.
 */
template<class FLOAT = float>
struct Mats
{
    /**
     * @brief    the floating point type for storing the statistic data
     */
    using FloatType = FLOAT;

    /**
     * @brief Create a empty Mats.
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
     * @brief    This function selects a region from the original matrices, and 
     *           assigns the selected submatrices to the data members respectively.
     * 
     * @param    r a region of interest
     */
    void roi(const cv::Rect& r) {
        mean   = mean   (r);
        stddev = stddev (r);
        cv     = cv     (r);
        num    = num    (r);
    }

    /**
     * @brief a matrix of mean values.
     * 
     */
    cv::Mat_<FLOAT>         mean    ;

    /**
     * @brief a matrix of standard deviations
     * 
     */
    cv::Mat_<FLOAT>         stddev  ;

    /**
     * @brief a matrix of coefficients of variation
     * 
     */
    cv::Mat_<FLOAT>         cv      ;

    /**
     * @brief a matrix of pixel numbers calculated per cell
     * 
     */
    cv::Mat_<std::uint32_t> num     ;
};

}
}