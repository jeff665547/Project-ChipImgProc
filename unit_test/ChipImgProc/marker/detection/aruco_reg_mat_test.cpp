#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/aruco_reg_mat.hpp>
#include "../../make_layout.hpp"

TEST(aruco_reg_mat, basic_test) {
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";

    // path to test images
    auto img0_path = nucleona::test::data_dir() / "aruco_test_img-0.tiff"; // in focus
    // auto img1_path = nucleona::test::data_dir() / "aruco_test_img-1.tiff"; // out of focus
    
    // load test images
    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    // auto img1 = cv::imread(img1_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);

    // path to marker frame template and mask
    auto frame_template_path = nucleona::test::data_dir() / "aruco_frame_template.tiff";
    auto frame_mask_path = nucleona::test::data_dir() / "aruco_frame_mask.tiff";

    // declare the ArUco markers that would be found in an image. Marker IDs can be inordered.
    std::vector<std::int32_t> aruco_ids_in_image {
        47, 48, 49, 05, 50, 51, 52,
        40, 41, 42, 43, 44, 45, 46,
        34, 35, 36, 37, 38, 39, 04,
        28, 29, 03, 30, 31, 32, 33,
        21, 22, 23, 24, 25, 26, 27,
        15, 16, 17, 18, 19, 02, 20,
        00, 01, 10, 11, 12, 13, 14
    };

    auto layout = make_banff_layout("banff_rc/pat_CY3.tsv", 2.4146);

    chipimgproc::marker::detection::ArucoRegMat reg_mat;
    reg_mat.set_dict(db_path.string(), "DICT_6X6_250");
    reg_mat.set_detector_ext(
        3, 1, 1, 13.4, 8.04, 
        frame_template_path.string(),
        frame_mask_path.string(), 
        9, 268, 5, 
        aruco_ids_in_image,
        3, 3,
        std::cout
    );

    auto mk_regs = reg_mat(
        static_cast<cv::Mat_<std::uint16_t>&>(img0), 
        layout, chipimgproc::MatUnit::PX, 
        std::cout
    );

    for(auto mk_r : mk_regs) {
        mk_r.info(std::cout);
    }
}