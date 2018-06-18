#include <ChipImgProc/rotation/estimate.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST(rotation_estimate, basic_test) {
    chipimgproc::rotation::Estimate<float> estimate;
    cv::Mat src;
    auto theta = estimate(src, false, cv::Mat(), 0.5, 0.5, 10, std::cout);
    std::cout << theta << std::endl;
}