#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/aruco.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <ChipImgProc/aruco/detector.hpp>
#include <ChipImgProc/aruco/dictionary.hpp>
TEST(aruco_test,basic_test) 
{
    std::vector<std::int32_t> aruco_ids_in_image({
        47, 48, 49, 05, 50, 51, 52,
        40, 41, 42, 43, 44, 45, 46,
        34, 35, 36, 37, 38, 39, 04,
        28, 29, 03, 30, 31, 32, 33,
        21, 22, 23, 24, 25, 26, 27,
        15, 16, 17, 18, 19, 02, 20,
        00, 01, 10, 11, 12, 13, 14
    });

    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    auto img0_path = nucleona::test::data_dir() / "aruco_test_img-0.tiff";
    auto img1_path = nucleona::test::data_dir() / "aruco_test_img-1.tiff";
    auto frame_template_path = nucleona::test::data_dir() / "aruco_frame_template.tiff";
    auto frame_mask_path = nucleona::test::data_dir() / "aruco_frame_mask.tiff";

    std::ifstream db_fin(db_path.string());
    nlohmann::json aruco_db;
    db_fin >> aruco_db;
    auto dict = chipimgproc::aruco::Dictionary::from_json(
        aruco_db["DICT_6X6_250"]
    );

    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    auto img1 = cv::imread(img1_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);

    auto frame_template = cv::imread(frame_template_path.string(), cv::IMREAD_GRAYSCALE);
    auto frame_mask     = cv::imread(frame_mask_path.string(), cv::IMREAD_GRAYSCALE);
    chipimgproc::aruco::Detector detector;
    detector.reset(
        dict, 
        3,                  // pyramid_level
        1,                  // border_bits
        1,                  // fringe_bits
        13.04,              // a_bit_width
        8.04,               // margin_size
        frame_template,     // locator image
        frame_mask,         // locator image mask
        9,                  // nms_count
        268,                // nms_radius
        5,                  // cell_size
        aruco_ids_in_image, // aruco ids
        std::cout           // logger
    );
    auto pts0 = detector.detect_markers(img0, std::cout);
    for(auto& [id, pt] : pts0 ) {
        std::cout << id << '\t' << pt << std::endl;
    }
    auto pts1 = detector.detect_markers(img1, std::cout);
    for(auto& [id, pt] : pts1 ) {
        std::cout << id << '\t' << pt << std::endl;
    }
}