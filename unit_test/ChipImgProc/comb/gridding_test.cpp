#include <ChipImgProc/comb/gridding.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>

TEST(comb_gridding, basic_test) {
    chipimgproc::comb::Gridding<float> gridder;
    gridder.set_logger(std::cout);
    gridder.set_grid_res_viewer([](const cv::Mat& m){
        cv::imwrite("debug_grid_result.tiff", m);
    });
    auto test_img_path = nucleona::test::data_dir() / "0-0-2.tiff";
    cv::Mat img = cv::imread(test_img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    chipimgproc::info(std::cout, img);
    auto res = gridder(img);
}