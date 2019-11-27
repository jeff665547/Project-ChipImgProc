/**
 * @file cell.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::stat::Cell
 * 
 */
#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
namespace chipimgproc { namespace stat{
/**
 * @brief The statistic data cell, contains mean value and standard deviation,
 *   coefficient of variation and pixel numbers. 
 * 
 * @tparam FLOAT The float point type, use to store the statistic data.
 */
template<class FLOAT = float>
struct Cell
{
    /**
     * @brief The float point type, use to store the statistic data.
     * 
     */
    using FloatType = FLOAT;

    /**
     * @brief Make statistic cell from cell data, 
     *   which is a sub-matrix from the rotation calibrated image.
     * 
     * @param mat Cell pixels. 
     * @return Cell Statistic data.
     */
    static Cell make(const cv::Mat& mat) {
        Cell res;
        cv::Scalar_<double> mean, stddev;
        cv::meanStdDev( mat, mean, stddev );
        
        res.mean    = mean(0);
        res.stddev  = stddev(0);
        res.cv      = res.stddev / res.mean;
        res.bg      = 0; 
        res.num     = mat.cols * mat.rows;
        return res;
    }

    /**
     * @brief mean of pixel values.
     */
    FLOAT mean;

    /**
     * @brief standard deviation of pixel values
     */
    FLOAT stddev;
    
    /**
     * @brief coefficient of variation of pixel values
     */
    FLOAT cv;

    /**
     * @brief background estimate
     */
    FLOAT bg;

    /**
     * @brief number of pixel values
     */
    std::uint32_t num;
};

}
}
