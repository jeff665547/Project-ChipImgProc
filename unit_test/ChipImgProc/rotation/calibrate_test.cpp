#include <ChipImgProc/rotation/calibrate.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST(rotation_calibrate, basic_test) {
    chipimgproc::rotation::Calibrate calibrate;
    cv::Mat src;
    calibrate(src, 50);
}