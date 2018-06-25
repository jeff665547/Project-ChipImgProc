#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
namespace chipimgproc { namespace stat{
struct Mats
{
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
    cv::Mat_<float>         mean    ;
    cv::Mat_<float>         stddev  ;
    cv::Mat_<float>         cv      ;
    cv::Mat_<std::uint32_t> num     ;
};

}
}