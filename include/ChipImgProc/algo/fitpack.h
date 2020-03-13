#pragma once
#include <tuple>
#include <vector>
#include <ChipImgProc/utils.h>

namespace chipimgproc::algo::fitpack {
std::tuple<
    std::vector<double>, // tx
    std::vector<double>, // ty
    std::vector<double>, // c
    std::vector<double>, // wrk
    int,                 // ier
    double
> surfit(
    std::vector<double>& x,
    std::vector<double>& y,
    std::vector<double>& z,
    std::vector<double>& w,
    double xb, double xe, 
    double yb, double ye,
    int kx, int ky,
    int iopt, double s,
    double eps,
    int nxest, int nyest,
    int lwrk1, int lwrk2
);
std::tuple< 
    std::vector<double>, // tx
    std::vector<double>, // ty
    std::vector<double>  // c
> bisplrep(
    std::vector<double>& x,
    std::vector<double>& y,
    std::vector<double>& z,
    int k = 3,
    std::optional<double> s = std::nullopt
);
cv::Mat_<double> bisplev(
    std::vector<double>& tx,
    std::vector<double>& ty,
    std::vector<double>& c,
    int k,
    std::vector<double>& x,
    std::vector<double>& y
);

}