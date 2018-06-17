#include <ChipImgProc/hough_transform.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST(hough_transform, basic_test) {
    chipimgproc::HoughTransform<float> hough_transform(0.1, 0.1, 3);
    cv::Mat mmm;
    auto hist = hough_transform(mmm);

}