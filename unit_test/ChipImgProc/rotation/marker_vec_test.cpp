/// [usage]
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include "../make_layout.hpp"

TEST(reg_mat_layout, operator_call_test) {
    /*
        In this example, we first detect marker location from the input image,
        and then use the MarkerVec to compute the rotation degree.
    */
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;

    // set image path
    auto img_path = 
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";

    // read image to matrix
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker layout
    auto mk_layout = make_zion_layout(2.68);
    
    // detect markers
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

    // compute rotation degree
    auto theta = marker_fit(mk_regs, std::cout);

    // output
    std::cout << theta << std::endl;
}
/// [usage]
TEST(reg_mat_layout, hard_case) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "202_20180612170327" / "0-1-CY3_1M.tiff";
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    auto mk_layout = make_zion_layout(2.68);
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    std::cout << theta << std::endl;
}