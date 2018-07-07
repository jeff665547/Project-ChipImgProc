#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
namespace chipimgproc { namespace stat{

template<class FLOAT = float>
struct Mats
{
    using FloatType = FLOAT;

    Mats() = default;
    Mats(int rows, int cols)
    : mean  (rows, cols)
    , stddev(rows, cols)
    , cv    (rows, cols)
    , num   (rows, cols)
    {}
    void roi(const cv::Rect& r) {
        mean   = mean   (r);
        stddev = stddev (r);
        cv     = cv     (r);
        num    = num    (r);
    }
    cv::Mat_<FLOAT>         mean    ;
    cv::Mat_<FLOAT>         stddev  ;
    cv::Mat_<FLOAT>         cv      ;
    cv::Mat_<std::uint32_t> num     ;
};

}
}