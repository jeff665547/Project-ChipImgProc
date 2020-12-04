#include <ChipImgProc/marker/detection/estimate_bias.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/aruco_random.hpp>
#include "../../make_layout.hpp"

TEST(estimate_bias_test, simultaion_shift) {
    const double um2px_r = 2.4145;
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    auto img_dir = nucleona::test::data_dir() / "aruco-green-pair";
    auto bf_path = img_dir / "0-1-BF.tiff";
    auto green_path = img_dir / "0-1-CY3.tiff";

    auto bf_img = cv::imread(bf_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    auto green_img = cv::imread(green_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    chipimgproc::info(std::cout, bf_img);
    chipimgproc::info(std::cout, green_img);

    // ArUco detection
    std::vector<std::int32_t> aruco_ids_in_image {
        47, 48, 49, 05, 50, 51, 52,
        40, 41, 42, 43, 44, 45, 46,
        34, 35, 36, 37, 38, 39, 04,
        28, 29, 03, 30, 31, 32, 33,
        21, 22, 23, 24, 25, 26, 27,
        15, 16, 17, 18, 19, 02, 20,
        00, 01, 10, 11, 12, 13, 14
    };
    std::vector<cv::Point> aruco_points(53);
    for(int y = 0; y < 7; y ++ ) {
        for(int x = 0; x < 7; x ++ ) {
            aruco_points[
                aruco_ids_in_image[y * 7 + x] 
            ] = cv::Point(x, y);
        }
    }
    auto [bf_templ, bf_mask] = chipimgproc::aruco::create_location_marker(
        50, 40, 3, 5, um2px_r
    );
    auto detector(chipimgproc::marker::detection::make_aruco_random(
        db_path.string(), 
        "DICT_6X6_250",
        bf_templ, bf_mask,
        30 * um2px_r, 
        2, 
        9, 
        50 * um2px_r, 
        0.75
    ));
    auto aruco_mk = detector(
        static_cast<cv::Mat_<std::uint8_t>&>(bf_img), 
        aruco_ids_in_image
    );
    // AM5B marker prepare
    auto mk_layout = make_banff_layout("banff_rc/pat_CY5.tsv", um2px_r);
    auto& green_templ = mk_layout.get_single_pat_marker_des().get_std_mk(chipimgproc::MatUnit::PX);
    auto& green_mask = mk_layout.get_single_pat_marker_des().get_std_mk_mask(chipimgproc::MatUnit::PX);
    chipimgproc::info(std::cout, green_templ);
    chipimgproc::info(std::cout, green_mask);

    // simulate bias between aruco and probe channel, Original is pos.x + 200, pos.y + 250 (*)
    std::vector<cv::Point2d> aruco_mk_pos;
    for(auto&& [mid, score, pos] : aruco_mk) {
        aruco_mk_pos.push_back(cv::Point2d(
            pos.x + 50,
            pos.y + 60
        ));
    }


    auto [bias, score] = chipimgproc::marker::detection::estimate_bias(
        green_img, green_templ, green_mask, aruco_mk_pos
    );
    std::cout << bias << std::endl;
    EXPECT_LT(std::abs(bias.x + 50), 3);
    EXPECT_LT(std::abs(bias.y + 60), 3);
}