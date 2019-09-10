#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
#include "make_layout.hpp"
int main() {
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
    auto img_path = "0-0-2.tiff";

    // read image
    cv::Mat_<std::uint16_t>img = cv::imread(
        img_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
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