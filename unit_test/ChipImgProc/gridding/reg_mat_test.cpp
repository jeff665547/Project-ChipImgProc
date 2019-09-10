/// [usage]
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
#include "../make_layout.hpp"
TEST(reg_mat_layout, operator_call_test) {
    /*
        In this example, we are going to explain how the gridding works.
        But by the chipimgproc::gridding::RegMat requirements, 
        the marker must be standardized regular matrix and 
        the image should be rotation calibrated.

        Therefore before we doing the gridding, we have to do some preprocess to the image.

        Here we use the ZION probe channel image as the input.
    */
    // create the tool we need.
    chipimgproc::marker::detection::RegMat reg_mat;     // for marker detection
    chipimgproc::rotation::MarkerVec<float> marker_fit; // for rotation inference
    chipimgproc::rotation::Calibrate rot_cali;          // for rotation calibration
    chipimgproc::gridding::RegMat gridding;             // for image gridding

    // set image path
    auto img_path = 
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";

    // read image
    cv::Mat_<std::uint16_t>img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // create ZION marker layout and set the micron to pixel rate to 2.68
    auto mk_layout = make_zion_layout(2.68);

    // detect markers
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

    // evaluate the rotation degree
    auto theta = marker_fit(mk_regs, std::cout);

    // rotation calibrate image
    rot_cali(img, theta);

    // re-detect the markers, 
    // because after the image rotation, the marker position is changed.
    mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

    // inference the missing marker and standardize the marker position.
    mk_regs = chipimgproc::marker::detection::reg_mat_infer(mk_regs, 0, 0, img);

    // do the gridding, and pass the debug viewer into algorithm. 
    auto gl_res = gridding(img, mk_layout, mk_regs, std::cout, [](const auto& m){
        cv:imwrite("debug_gridding.tiff", m);
    });
    
    // the gl_res contains all workable grid lines, 
    // and "debug_gridding.tiff" image will save in the working directory.
    
    // check debug_gridding.tiff for visable gridding result.
}
/// [usage]

auto pat_img(const std::string& id) {
    auto zion_pat_px_path = nucleona::test::data_dir() / id;
    auto mk_px_ = cv::imread(zion_pat_px_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    chipimgproc::info(std::cout, mk_px_);
    cv::Mat_<std::uint8_t> mk_px;
    chipimgproc::info(std::cout, mk_px);
    cv::extractChannel(mk_px_, mk_px, mk_px_.channels() - 1);
    return mk_px;

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