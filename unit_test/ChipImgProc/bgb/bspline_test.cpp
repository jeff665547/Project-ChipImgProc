#include <ChipImgProc/bgb/bspline.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>

using namespace chipimgproc;
TEST(bgb, bspline) {
    auto img_path = nucleona::test::data_dir() / "banff_test" / "1-1-2.tiff";
    auto img = imread(img_path);
    auto org = img.clone();
    chipimgproc::bgb::BSpline bspline;
    auto surf = bspline(img, {3, 6}, 0.3);
    img.forEach<std::uint16_t>([&surf](std::uint16_t& v, const int pos[]){
        const auto& r = pos[0];
        const auto& c = pos[1];
        v = std::round(std::max(0.0, v - surf(r, c)));
    });
    cv::Rect high(1460, 1640, 240, 240);
    cv::Rect low(80, 80, 240, 240);
    auto x = img.cols / 2;
    auto y = img.rows / 2;
    double v = cv::mean(surf(high))[0] - cv::mean(surf(low))[0];
    std::cout << v << std::endl;
    EXPECT_GT(v, 0);
}