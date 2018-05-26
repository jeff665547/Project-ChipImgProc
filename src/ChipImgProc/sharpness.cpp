#include <ChipImgProc/sharpness.h>
#include <ChipImgProc/utils.h>
namespace chipimgproc {

double sharpness( const cv::Mat& m ) {
    cv::Mat dst;
    cv::Mat tmp(m.rows, m.cols, m.type());
    cv::Laplacian(m, tmp, m.type());
    absdiff(tmp, cv::Scalar::all(0), dst);
    double sv = cv::sum(dst)[0];
    return sv;
}

}