/// [usage]
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/aruco_reg_mat.hpp>
#include "../../make_layout.hpp"
TEST(aruco_reg_mat, basic_test) {
    // In this using case, we are going to recognize the ArUco marker from the image.
    // The input is a Banff FOV image and the output is the ArUco marker regions (a vector of cv::Rect)

    // set aruco database path
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";

    // set sample input image path
    auto img0_path = nucleona::test::data_dir() / "aruco_test_img-0.tiff"; // in focus
    // auto img1_path = nucleona::test::data_dir() / "aruco_test_img-1.tiff"; // out of focus
    
    // load test images
    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    // auto img1 = cv::imread(img1_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);

    // set path to marker border frame template and mask
    auto frame_template_path = nucleona::test::data_dir() / "aruco_frame_template.tiff";
    auto frame_mask_path = nucleona::test::data_dir() / "aruco_frame_mask.tiff";

    // Set candidate marker id list.
    // We hard code the Banff ArUco marker ids in this example.
    // Note that this list contains all markers in the Baff chip, not only the FOV
    std::vector<std::int32_t> aruco_ids_in_image {
        47, 48, 49, 05, 50, 51, 52,
        40, 41, 42, 43, 44, 45, 46,
        34, 35, 36, 37, 38, 39, 04,
        28, 29, 03, 30, 31, 32, 33,
        21, 22, 23, 24, 25, 26, 27,
        15, 16, 17, 18, 19, 02, 20,
        00, 01, 10, 11, 12, 13, 14
    };

    // aruco_points is a marker id -> marker point table
    // marker point is the marker sequencial point, not the absolute location on image
    // For example, consider the aruco_ids_in_image, we permute the marker id into a matrix.
    // The matrix location is marker sequencial point, therefore 
    // marker id    marker point(x,y)
    //        47 -> (0,0)
    //        48 -> (1,0)
    // ...
    std::vector<cv::Point> aruco_points(53);
    for(int y = 0; y < 7; y ++ ) {
        for(int x = 0; x < 7; x ++ ) {
            aruco_points[
                aruco_ids_in_image[y * 7 + x] 
            ] = cv::Point(x, y);
        }
    }

    // Make marker layout
    // To make Banff marker layout, we need many parameters depend on chip specification
    // Currently we hard code these parameters to make example works
    // The detail Banff layout making process can be found in unit_test/ChipImgProc/make_layout.hpp
    
    // The path "banff_rc/pat_CY3.tsv" is the probe channel marker pattern, 
    // which is not used in this example we just randomly choose a pattern to make it works
    
    // The magic number 2.68 is the micron to pixel rate, which depend on image and reader.
    // Currently we hard code this parameter for example.
    auto layout = make_banff_layout("banff_rc/pat_CY3.tsv", 2.68);

    // Create the detector
    chipimgproc::marker::detection::ArucoRegMat reg_mat;

    // Set database and dictionary name
    reg_mat.set_dict(db_path.string(), "DICT_6X6_250");

    // Set the detector parameter.
    // These parameters are highly depend on chip specification.
    // The means of these parameters can be found in API document, now we just hard code in this example.
    reg_mat.set_detector_ext(
        3, 1, 1, 13.4, 8.04, 
        frame_template_path.string(),
        frame_mask_path.string(), 
        9, 268, 5, 
        aruco_ids_in_image,
        aruco_points,
        std::cout
    );

    // Call the detector
    // Now we detect markers in pixel domain, so the matrix unit we use PX
    auto mk_regs = reg_mat(
        static_cast<cv::Mat_<std::uint8_t>&>(img0), 
        layout, chipimgproc::MatUnit::PX, 
        std::cout
    );

    // Print the marker region location
    // Each output row is 
    // (marker location)[marker height/width](marker point id)
    for(auto mk_r : mk_regs) {
        mk_r.info(std::cout);
    }
    // Due to the image quality variant, it may not detect all markers
    // For image gridding process, it should detect 2 marker in deferent column and row at least.
}
/// [usage]