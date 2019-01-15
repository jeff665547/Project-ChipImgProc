#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
namespace chipimgproc { namespace stat{

template<class FLOAT = float>
struct Cell
{
    using FloatType = FLOAT;

    static Cell make(const cv::Mat& mat) {
        Cell res;
        cv::Scalar_<double> mean, stddev;
        cv::meanStdDev( mat, mean, stddev );
        
        res.mean    = mean(0);
        res.stddev  = stddev(0);
        res.cv      = res.stddev / res.mean;
        res.num     = mat.cols * mat.rows;
        return res;
    }
    FLOAT mean;
    FLOAT stddev;
    FLOAT cv;
    std::uint32_t num;
};

}
}