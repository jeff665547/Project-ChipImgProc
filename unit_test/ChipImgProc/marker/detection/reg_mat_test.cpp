#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include "../../make_layout.hpp"

TEST(reg_mat_layout, operator_call_test) {
    chipimgproc::marker::detection::RegMat reg_mat;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    auto mk_layout = make_zion_layout(2.68);
    reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0,
        std::cout, 
        [](const cv::Mat& mat){
            cv::imwrite("debug_bin.tiff", mat);
        }, 
        [](const cv::Mat& mat){
            cv::imwrite("debug_search.tiff", mat);
        }, 
        [](const cv::Mat& mat){
            cv::imwrite("debug_marker.tiff", mat);
        });
}


TEST(reg_mat_layout, hard_case) {
    chipimgproc::marker::detection::RegMat reg_mat;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "202_20180612170327" / "0-1-CY3_1M.tiff";
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    auto mk_layout = make_zion_layout(2.41);
    reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, 
        std::cout, 
        [](const cv::Mat& mat){
            cv::imwrite("debug_bin.tiff", mat);
        }, 
        [](const cv::Mat& mat){
            cv::imwrite("debug_search.tiff", mat);
        }, 
        [](const cv::Mat& mat){
            cv::imwrite("debug_marker.tiff", mat);
        });
}

