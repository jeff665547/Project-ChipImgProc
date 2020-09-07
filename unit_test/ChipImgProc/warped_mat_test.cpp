#include <ChipImgProc/warped_mat.hpp>
#include <ChipImgProc/multi_warped_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/aruco_random.hpp>
#include <range/v3/view.hpp>
#include <ChipImgProc/marker/detection/estimate_bias.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/warped_mat/estimate_transform_mat.hpp>
using namespace chipimgproc;

TEST(warped_mat_test, basic_test) {
    using namespace std::string_literals;
    const double um2px_r = 2.4145;
    const int rescale = 2;
    const double rescaled_um2px_r = um2px_r / rescale;
    double mk_xi_um = 0            ;
    double mk_yi_um = 0            ;
    int mk_w_d_um   = 405 * rescale;
    int mk_h_d_um   = 405 * rescale;
    int mk_w_um     = 50  * rescale;
    int mk_h_um     = 50  * rescale;
    int cl_w_um     = 4   * rescale;
    int cl_h_um     = 4   * rescale;
    int sp_w_um     = 1   * rescale;
    int sp_h_um     = 1   * rescale;
    int cl_wd_um    = cl_w_um + sp_w_um;
    int cl_hd_um    = cl_h_um + sp_h_um;
    int cl_wn       = 172;
    int cl_hn       = 172;
    double win_r    = 0.6;

    auto pb_mk_path = nucleona::test::data_dir() / "banff_rc" / "pat_CY5.tsv";
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    auto img0_path = nucleona::test::data_dir() / "aruco-green-pair" / "0-1-BF.tiff"; // in focus
    auto img1_path = nucleona::test::data_dir() / "aruco-green-pair" / "0-1-CY3.tiff"; // in focus
    auto img0 = cv::imread(img0_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    auto img1 = cv::imread(img1_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
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
        50, 40, 3, 5, um2px_r
    );

    // prepare pixel domain anchors
    auto detector(marker::detection::make_aruco_random(
        db_path.string(), 
        "DICT_6X6_250",
        templ, mask,
        30 * um2px_r, 
        2, 
        9, 
        50 * um2px_r, 
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
    auto trans_mat = warped_mat::estimate_transform_mat(um_pos, px_pos);
    std::cout << trans_mat << std::endl;
    
    // probe channel process
    auto [pb_templ, pb_mask] = marker::Loader::from_file_to_img(
        pb_mk_path.string(), 
        cl_h_um, // * rescaled_um2px_r,
        cl_w_um, // * rescaled_um2px_r,
        sp_w_um, // * rescaled_um2px_r
        um2px_r
    );
    auto [bias, score] = marker::detection::estimate_bias(img1, pb_templ, pb_mask, px_pos, trans_mat);
    auto probe_trans_mat = trans_mat.clone();
    {
        auto _bias = bias;
        typed_mat(probe_trans_mat, [&_bias](auto& mat){
            mat(0, 2) += _bias.x;
            mat(1, 2) += _bias.y;
        });
    }

    std::vector<cv::Point2d> img_px_pos;
    for(auto&& p : px_pos) {
        img_px_pos.push_back(p + bias);
    }
    chipimgproc::ip_convert(img1, CV_32F);
    auto warped_mat = make_warped_mat(
        probe_trans_mat, img1, 
        {mk_xi_um, mk_yi_um},
        cl_w_um, cl_h_um,
        cl_wd_um, cl_hd_um,
        cl_wn * cl_wd_um,
        cl_hn * cl_hd_um,
        win_r,
        rescaled_um2px_r,
        cl_wn, cl_hn
    );
    cv::Mat_<std::uint8_t> first_marker(10, 10);
    for(int i = 0; i < 10; i ++) {
        for(int j = 0; j < 10; j ++) {
            // first_marker(i, j) = cv::mean(warped_mat.at_cell(i, j).patch)[0] / 64;
            first_marker(i, j) = warped_mat.at_cell(i, j).mean / 64;
        }
    }
    first_marker = binarize(first_marker);
    cv::imwrite("test.png", first_marker);
    std::vector<int> ans ({
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 1, 0, 0, 0, 0, 0, 0, 1, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 1, 1, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    });
    for(int i = 0; i < 10; i ++) {
        for(int j = 0; j < 10; j ++) {
            EXPECT_EQ(ans[(i * 10) + j] * 255, first_marker(i, j));
        }
    }

}