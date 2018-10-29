#include <fstream>
#include <nlohmann/json.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/aruco.hpp>
TEST(aruco_test,basic_test) 
{
/// [usage]

    // Please include the following headers to your C++ code
    // * nlohmann/json.hpp
    // * ChipImgProc/utils.h
    // * ChipImgProc/aruco.hpp

    /*
     * prepare test data
     */

    // path to test images
    auto img0_path = nucleona::test::data_dir() / "aruco_test_img-0.tiff"; // in focus
    auto img1_path = nucleona::test::data_dir() / "aruco_test_img-1.tiff"; // out of focus
    
    // load test images
    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    auto img1 = cv::imread(img1_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);

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
    
    /*
     * prepare marker detection materials
     */
    
    // path to ArUco coding database
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    
    // load ArUco dictionary, which is composed by 250 markers and a marker size of 6x6 bits
    std::ifstream db_fin(db_path.string());
    nlohmann::json aruco_db;
    db_fin >> aruco_db;
    auto dict = chipimgproc::aruco::Dictionary::from_json(
        aruco_db["DICT_6X6_250"]
    );
    
    // path to marker frame template and mask
    auto frame_template_path = nucleona::test::data_dir() / "aruco_frame_template.tiff";
    auto frame_mask_path = nucleona::test::data_dir() / "aruco_frame_mask.tiff";

    // load marker frame template and mask
    auto frame_template = cv::imread(frame_template_path.string(), cv::IMREAD_GRAYSCALE);
    auto frame_mask     = cv::imread(frame_mask_path.string(), cv::IMREAD_GRAYSCALE);
    
    // declare an aruco marker detector
    chipimgproc::aruco::Detector detector;
    detector.reset(
        dict, 
        3,                  // pyramid_level
        1,                  // border_bits
        1,                  // fringe_bits
        13.4,               // a_bit_width
        8.04,               // margin_size
        frame_template,
        frame_mask,
        9,                  // nms_count
        268,                // nms_radius
        5,                  // cell_size
        aruco_ids_in_image, // a list of candidate marker ids
        std::cout           // logger
    );
    
    /*
     * detect the markers for test images, and 
     * show up recognized marker IDs and corresponding center positions
     */
    
    std::cout << "aruco_test_img-0.tiff" << std::endl;
    auto pts0 = detector.detect_markers(img0, std::cout);
    for(auto& [id, pt] : pts0 )
        std::cout << id << '\t' << pt << std::endl;
    
    std::cout << "aruco_test_img-1.tiff\n"  << std::endl;
    auto pts1 = detector.detect_markers(img1, std::cout);
    for(auto& [id, pt] : pts1 )
        std::cout << id << '\t' << pt << std::endl;
    
/// [usage]
}