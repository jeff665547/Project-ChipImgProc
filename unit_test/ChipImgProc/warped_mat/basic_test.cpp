#include <ChipImgProc/warped_mat/basic.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/aruco_random.hpp>
#include <opencv2/calib3d.hpp>
using namespace chipimgproc::warped_mat;
using namespace chipimgproc;
TEST(basic_warped_mat_test, basic_test) {
    using namespace std::string_literals;
    int mk_xi_um = 0;
    int mk_yi_um = 0;
    int mk_w_d_um = 405;
    int mk_h_d_um = 405;
    int mk_w_um = 50;
    int mk_h_um = 50;
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    auto img0_path = nucleona::test::data_dir() / "aruco-green-pair" / "0-1-BF.tiff"; // in focus
    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    std::vector<std::int32_t> aruco_ids_in_image {
        47, 48, 49, 05, 50, 51, 52,
        40, 41, 42, 43, 44, 45, 46,
        34, 35, 36, 37, 38, 39, 04,
        28, 29, 03, 30, 31, 32, 33,
        21, 22, 23, 24, 25, 26, 27,
        15, 16, 17, 18, 19, 02, 20,
        00, 01, 10, 11, 12, 13, 14
    };
    nlohmann::json aruco_ids_map;
    for(std::size_t y = 0; y < 7; y ++) {
        for(std::size_t x = 0; x < 7; x ++) {
            auto key = aruco_ids_in_image.at((y * 7) + x);
            aruco_ids_map[std::to_string(key)] = {x, y};
        }
    }
    std::vector<cv::Point> aruco_points(53);
    for(int y = 0; y < 7; y ++ ) {
        for(int x = 0; x < 7; x ++ ) {
            aruco_points[
                aruco_ids_in_image[y * 7 + x] 
            ] = cv::Point(x, y);
        }
    }
    auto [templ, mask] = aruco::create_location_marker(
        50, 40, 3, 5, 2.4145
    );

    // prepare pixel domain anchors
    auto detector(marker::detection::make_aruco_random(
        db_path.string(), 
        "DICT_6X6_250",
        templ, mask,
        30 * 2.4145, 
        2, 
        9, 
        50 * 2.4145, 
        0.75
    ));
    auto mk_regs = detector(
        static_cast<cv::Mat_<std::uint8_t>&>(img0), 
        aruco_ids_in_image
    );

    // prepare um domain anchors
    aruco::MarkerMap mk_map(aruco_ids_map);
    std::vector<cv::Point2d> px_pos;
    std::vector<cv::Point2d> um_pos;
    for(auto [mid, score, loc] : mk_regs) {
        px_pos.push_back(loc);
        auto mkpid = mk_map.get_sub(mid);
        auto mk_x_um = ((mkpid.x - 2) * mk_w_d_um) + mk_xi_um + (mk_w_um / 2);
        auto mk_y_um = (mkpid.y * mk_h_d_um) + mk_yi_um + (mk_h_um / 2);
        um_pos.emplace_back(mk_x_um, mk_y_um);
    }
    auto trans_mat = cv::estimateAffinePartial2D(um_pos, px_pos);

    auto warped_mat = make_basic(
        trans_mat, 
        {img0}, 
        {0, 0},
        5, 5,
        860, 860
    );
    EXPECT_EQ(warped_mat.rows(), 172);
    EXPECT_EQ(warped_mat.cols(), 172);
    cv::Mat_<std::uint8_t> first_marker(10, 10);
    for(int i = 0; i < 10; i ++) {
        for(int j = 0; j < 10; j ++) {
            first_marker(i, j) = cv::mean(warped_mat.at_cell(i, j, 0))[0];
        }
    }
    first_marker = binarize(first_marker);
    cv::imwrite("test.png", first_marker);
    std::vector<int> ans ({
        1, 0, 0, 1, 0, 0
      , 1, 1, 1, 1, 1, 0
      , 1, 0, 1, 1, 0, 1
      , 1, 1, 1, 0, 0, 0
      , 1, 0, 1, 1, 0, 0
      , 0, 1, 0, 1, 0, 0
    });
    for(int i = 2; i < 8; i ++) {
        for(int j = 2; j < 8; j ++) {
            EXPECT_EQ(ans[((i-2) * 6) + (j - 2)] * 255, first_marker(i, j));
        }
    }
}
