/// [usage]
#include <fstream>
#include <nlohmann/json.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/aruco.hpp>
TEST(aruco_test,basic_test) 
{
    // In this using case, we are going to recognize the ArUco marker from the image.
    // The input is 2 Banff FOV images and the output is the ArUco marker centor points.

    // set sample input image path
    auto img0_path = nucleona::test::data_dir() / "aruco_test_img-0.tiff"; // in focus
    auto img1_path = nucleona::test::data_dir() / "aruco_test_img-1.tiff"; // out of focus
    
    // load test images
    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    auto img1 = cv::imread(img1_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);

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
    
    
    // set aruco database path
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    
    // load ArUco dictionary, which is composed by 250 markers and a marker size of 6x6 bits
    std::ifstream db_fin(db_path.string());
    nlohmann::json aruco_db;
    db_fin >> aruco_db;
    auto dict = chipimgproc::aruco::Dictionary::from_json(
        aruco_db["DICT_6X6_250"] // dictionary name
    );
    
    // set path to marker border frame template and mask
    auto frame_template_path = nucleona::test::data_dir() / "aruco_frame_template.tiff";
    auto frame_mask_path = nucleona::test::data_dir() / "aruco_frame_mask.tiff";

    // load marker frame template and mask as gray scale image
    auto frame_template = cv::imread(frame_template_path.string(), cv::IMREAD_GRAYSCALE);
    auto frame_mask     = cv::imread(frame_mask_path.string(), cv::IMREAD_GRAYSCALE);
    
    // Create the detector
    chipimgproc::aruco::Detector detector;

    // Set the detector parameter.
    // These parameters are highly depend on chip specification.
    // The means of these parameters can be found in API document, now we just hard code in this example.
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

    // Call the detector
    // The output is marker centor point
    std::cout << "aruco_test_img-0.tiff" << std::endl;
    auto pts0 = detector.detect_markers(img0, std::cout);
    for(auto& [id, pt] : pts0 )
        std::cout << id << '\t' << pt << std::endl;
    
    std::cout << "aruco_test_img-1.tiff\n"  << std::endl;
    auto pts1 = detector.detect_markers(img1, std::cout);
    for(auto& [id, pt] : pts1 )
        std::cout << id << '\t' << pt << std::endl;
    
}
/// [usage]