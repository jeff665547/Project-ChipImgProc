#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include "../make_layout.hpp"
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
auto pat_img(const std::string& id) {
    auto zion_pat_px_path = nucleona::test::data_dir() / id;
    auto mk_px_ = cv::imread(zion_pat_px_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    chipimgproc::info(std::cout, mk_px_);
    cv::Mat_<std::uint8_t> mk_px;
    chipimgproc::info(std::cout, mk_px);
    cv::extractChannel(mk_px_, mk_px, mk_px_.channels() - 1);
    return mk_px;

}
TEST(reg_mat_layout, operator_call_test) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;
    chipimgproc::rotation::Calibrate rot_cali;
    chipimgproc::gridding::RegMat gridding;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";
    cv::Mat_<std::uint16_t>img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    auto mk_layout = make_zion_layout(2.68);
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    rot_cali(img, theta);
    mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);
    mk_regs = chipimgproc::marker::detection::reg_mat_infer(mk_regs, 0, 0, img);
    auto gl_res = gridding(img, mk_layout, mk_regs, std::cout, [](const auto& m){
        cv:imwrite("debug_gridding.tiff", m);
    });
}

TEST(reg_mat_layout, hard_case) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;
    chipimgproc::rotation::Calibrate rot_cali;
    chipimgproc::gridding::RegMat gridding;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "202_20180612170327" / "0-1-CY3_1M.tiff";
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    auto mk_layout = make_zion_layout(2.41);
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    rot_cali(img, theta);
    mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);
    mk_regs = chipimgproc::marker::detection::reg_mat_infer(mk_regs, 0, 0, img);
    auto gl_res = gridding(img, mk_layout, mk_regs, std::cout, [](const auto& m){
        cv:imwrite("debug_gridding.tiff", m);
    });
}