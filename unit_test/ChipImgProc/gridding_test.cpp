#include <ChipImgProc/gridding.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
TEST(gridding, eyebal_test) {
    chipimgproc::Gridding<float> gridder;
    auto test_img_path = nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";
    cv::Mat_<float> img = cv::imread(test_img_path.string());
    auto grid_res = gridder(
        img, 
        35,
        std::cout,
        []( const cv::Mat& m) {
            cv::imwrite(
                (nucleona::test::data_dir() / "gridding_eyeball_test_0.tiff").string(),
                m
            );
        }
    );
}