/**
 * @file mats.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::stat::Mats
 * 
 */

#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include "cell.hpp"
namespace chipimgproc { namespace stat{

/**
 * @brief    This Mats class contains a collection of matrices 
 *           for storing the mean values, standard deviations, 
 *           coefficients of variation, and the amount of pixels 
 *           for summarization for each grid cell.
 * 
 * @tparam   FLOAT denotes the floating point variable type.
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
    : mean  (cv::Mat_<FLOAT>::zeros(rows, cols))
    , stddev(cv::Mat_<FLOAT>::zeros(rows, cols))
    , cv    (cv::Mat_<FLOAT>::zeros(rows, cols))
    , bg    (cv::Mat_<FLOAT>::zeros(rows, cols))
    , num   (cv::Mat_<std::uint32_t>::zeros(rows, cols))
    , min_cv_pos(cv::Mat_<cv::Point2d>::zeros(rows, cols))
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
        bg     = bg     (r);
        num    = num    (r);
    }

    Cell<FLOAT> operator()(int r, int c) const {
        Cell<FLOAT> res;
        res.mean    = mean      (r, c);
        res.stddev  = stddev    (r, c);
        res.cv      = cv        (r, c);
        res.bg      = bg        (r, c);
        res.num     = num       (r, c);
        return res;
    }
    int rows() const { return mean.rows; }
    int cols() const { return mean.cols; }

    /**
     * @brief a matrix of mean values.
     */
    cv::Mat_<FLOAT> mean;

    /**
     * @brief a matrix of standard deviations
     */
    cv::Mat_<FLOAT> stddev;

    /**
     * @brief a matrix of coefficients of variation
     */
    cv::Mat_<FLOAT> cv;

    /**
     * @brief a matrix of background estimates
     */
    cv::Mat_<FLOAT> bg;

    /**
     * @brief a matrix of pixel numbers for a cell summarization
     */
    cv::Mat_<std::uint32_t> num;

    /**
     * @brief a matrix of position of min-cv
     **/
    cv::Mat_<cv::Point2d> min_cv_pos;
};

}
}
