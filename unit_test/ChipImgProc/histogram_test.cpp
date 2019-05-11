#include <ChipImgProc/histogram.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
TEST(histogram, basic_test) {
    auto img_path = nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";
    auto img = chipimgproc::imread(img_path);
    auto hist = chipimgproc::histogram(cv::Mat_<std::uint16_t>(img));
}