#include <ChipImgProc/rotation/line_detection.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST(rotation_line_detection, basic_test) {
    chipimgproc::rotation::LineDetection<float> line_detection;
    cv::Mat src;
    auto theta = line_detection(src, false, cv::Mat(), 0.5, 0.5, 10, std::cout);
    std::cout << theta << std::endl;
}