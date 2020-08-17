#include <ChipImgProc/warped_mat.hpp>
#include <ChipImgProc/multi_warped_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/aruco_random.hpp>
#include <range/v3/view.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/marker/detection/estimate_bias.hpp>
#include <ChipImgProc/warped_mat/estimate_transform_mat.hpp>
using namespace chipimgproc;

TEST(multi_warped_mat_test, with_basic_test) {
    using namespace std::string_literals;
    const double um2px_r = 2.4145;
    const int rescale = 1;
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
    // int mk_xi_um = 0;
    // int mk_yi_um = 0;
    // int mk_w_d_um = 405;
    // int mk_h_d_um = 405;
    // int mk_w_um = 50;
    // int mk_h_um = 50;
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    std::vector<boost::filesystem::path> img_paths({
        nucleona::test::data_dir() / "aruco-green-pair" / "0-0-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "0-1-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "0-2-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-0-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-1-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-2-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-0-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-1-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-2-BF.tiff" 
    });
    std::vector<cv::Point2d> stitch_point({
        {0, 0   }, {810, 0   }, {1620, 0   },
        {0, 810 }, {810, 810 }, {1620, 810 },
        {0, 1620}, {810, 1620}, {1620, 1620}
    });
    std::vector<cv::Point> mk_bias({
        {0, 0}, {2, 0}, {4, 0},
        {0, 2}, {2, 2}, {4, 2},
        {0, 4}, {2, 4}, {4, 4}

    });
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
    aruco::MarkerMap mk_map(aruco_ids_map);
    std::vector<BasicWarpedMat<true>> warped_mats;
    for(auto&& [img_p, st_p, mk_b] : ranges::view::zip(img_paths, stitch_point, mk_bias)) {
        auto img = ::imread(img_p);
        auto mk_regs = detector(
            static_cast<cv::Mat_<std::uint8_t>&>(img), 
            aruco_ids_in_image
        );

        // prepare um domain anchors
        std::vector<cv::Point2d> px_pos;
        std::vector<cv::Point2d> um_pos;
        for(auto [mid, score, loc] : mk_regs) {
            px_pos.push_back(loc);
            auto mkpid = mk_map.get_sub(mid);
            auto mk_x_um = (mkpid.x * mk_w_d_um) + mk_xi_um + (mk_w_um / 2) - st_p.x;
            auto mk_y_um = (mkpid.y * mk_h_d_um) + mk_yi_um + (mk_h_um / 2) - st_p.y;
            um_pos.emplace_back(mk_x_um, mk_y_um);
        }
       //  auto trans_mat = cv::estimateAffinePartial2D(um_pos, px_pos);
        auto trans_mat = warped_mat::estimate_transform_mat(um_pos, px_pos);

        auto warped_mat = make_basic_warped_mat(
            trans_mat, 
            {img}, 
            {mk_xi_um, mk_yi_um},
            5, 5,
            860, 860
        );
        warped_mats.emplace_back(std::move(warped_mat));
    }
    auto multi_warped_mat = make_multi_warped_mat(
        std::move(warped_mats),  std::move(stitch_point),
        {0, 0}, 5, 5, 2480, 2480
    );
    // {
    //     auto src = ::imread(img_paths[0]);
    //     cv::Mat_<std::uint8_t> first_marker(10, 10);
    //     for(int i = 0; i < 10; i ++) {
    //         for(int j = 0; j < 10; j ++) {
    //             auto&& cell = multi_warped_mat.at_cell(i, j);
    //             first_marker(i, j) = cell.mean;
    //             cv::drawMarker(src, cv::Point(
    //                 std::round(cell.img_p.x),
    //                 std::round(cell.img_p.y)
    //             ), cv::Scalar(0, 0, 0), cv::MARKER_CROSS);
    //         }
    //     }
    //     std::cout << first_marker << '\n';
    //     first_marker = std::get<1>(threshold(first_marker, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU));
    //     cv::imwrite("testx.png", first_marker);
    //     cv::imwrite("testy.png", src);

    //     // std::vector<int> ans ({
    //     //     1, 0, 0, 1, 0, 0
    //     //   , 1, 1, 1, 1, 1, 0
    //     //   , 1, 0, 1, 1, 0, 1
    //     //   , 1, 1, 1, 0, 0, 0
    //     //   , 1, 0, 1, 1, 0, 0
    //     //   , 0, 1, 0, 1, 0, 0
    //     // });
    //     // for(int i = 2; i < 8; i ++) {
    //     //     for(int j = 2; j < 8; j ++) {
    //     //         EXPECT_EQ(ans[((i-2) * 6) + (j - 2)] * 255, first_marker(i, j));
    //     //     }
    //     // }
    // }
    {
        cv::Mat_<std::uint8_t> first_marker(10, 10);
        for(int i = 0; i < 10; i ++) {
            for(int j = 0; j < 10; j ++) {
                first_marker(i, j) = multi_warped_mat.at_cell(i, j + 162).mean;
            }
        }
        // first_marker = binarize(first_marker);
        first_marker = std::get<1>(threshold(first_marker, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU));
        cv::imwrite("test0.png", first_marker);
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
    {
        cv::Mat_<std::uint8_t> first_marker(10, 10);
        for(int i = 0; i < 10; i ++) {
            for(int j = 0; j < 10; j ++) {
                first_marker(i, j) = multi_warped_mat.at_cell(i, j + 324).mean;
            }
        }
        // first_marker = binarize(first_marker);
        first_marker = std::get<1>(threshold(first_marker, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU));
        cv::imwrite("test1.png", first_marker);
        std::vector<int> ans ({
            1, 0, 0, 1, 1, 0
          , 0, 0, 1, 0, 0, 0
          , 1, 1, 0, 1, 1, 0
          , 1, 0, 1, 0, 0, 0
          , 0, 1, 0, 0, 1, 1
          , 0, 1, 0, 1, 0, 0
        });
        for(int i = 2; i < 8; i ++) {
            for(int j = 2; j < 8; j ++) {
                EXPECT_EQ(ans[((i-2) * 6) + (j - 2)] * 255, first_marker(i, j));
            }
        }
    }
    {
        cv::Mat_<std::uint8_t> first_marker(10, 10);
        for(int i = 0; i < 10; i ++) {
            for(int j = 0; j < 10; j ++) {
                first_marker(i, j) = multi_warped_mat.at_cell(i + 162, j + 162).mean;
            }
        }
        first_marker = binarize(first_marker);
        cv::imwrite("test2.png", first_marker);
        std::vector<int> ans ({
            0, 0, 1, 1, 1, 0
          , 1, 0, 0, 1, 0, 0
          , 0, 0, 0, 1, 0, 1
          , 1, 1, 1, 1, 1, 0
          , 1, 1, 1, 0, 1, 0
          , 0, 1, 1, 1, 1, 0
        });
        for(int i = 2; i < 8; i ++) {
            for(int j = 2; j < 8; j ++) {
                EXPECT_EQ(ans[((i-2) * 6) + (j - 2)] * 255, first_marker(i, j));
            }
        }

    }
}

TEST(multi_warped_mat_test, with_mask_warped_mat_test) {
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
    int fov_wd_cl   = 162;
    int fov_hd_cl   = 162;
    int fov_rows    = 3;
    int fov_cols    = 3;
    int mk_wd_cl    = 10;
    int mk_hd_cl    = 10;
    double win_r    = 0.6;
    // int mk_xi_um = 0;
    // int mk_yi_um = 0;
    // int mk_w_d_um = 405;
    // int mk_h_d_um = 405;
    // int mk_w_um = 50;
    // int mk_h_um = 50;
    auto pb_mk_path = nucleona::test::data_dir() / "banff_rc" / "pat_CY5.tsv";
    auto db_path = nucleona::test::data_dir() / "aruco_db.json";
    std::vector<boost::filesystem::path> img_paths({
        nucleona::test::data_dir() / "aruco-green-pair" / "0-0-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "0-1-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "0-2-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-0-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-1-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-2-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-0-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-1-BF.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-2-BF.tiff" 
    });
    std::vector<boost::filesystem::path> probe_img_paths({
        nucleona::test::data_dir() / "aruco-green-pair" / "0-0-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "0-1-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "0-2-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-0-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-1-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "1-2-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-0-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-1-CY3.tiff",
        nucleona::test::data_dir() / "aruco-green-pair" / "2-2-CY3.tiff" 
    });
    // std::vector<cv::Point2d> stitch_point({
    //     {0, 0   }, {810, 0   }, {1620, 0   },
    //     {0, 810 }, {810, 810 }, {1620, 810 },
    //     {0, 1620}, {810, 1620}, {1620, 1620}
    // });
    std::vector<cv::Point> mk_bias({
        {0, 0}, {2, 0}, {4, 0},
        {0, 2}, {2, 2}, {4, 2},
        {0, 4}, {2, 4}, {4, 4}

    });
    std::vector<cv::Point2d> stitch_point;
    for(auto&& mkb : mk_bias) {
        stitch_point.emplace_back(
            mkb.x * mk_w_d_um,
            mkb.y * mk_h_d_um
        );
    }
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
    aruco::MarkerMap mk_map(aruco_ids_map);
    std::vector<WarpedMat<true, float>> warped_mats;
    // warped_mats.reserve(img_paths.size());
    for(auto&& [img_p, st_p, mk_b, pb_img_p] : ranges::view::zip(
        img_paths, stitch_point, 
        mk_bias, probe_img_paths
    )) {
        auto img = ::imread(img_p);
        auto pb_img = ::imread(pb_img_p);
        auto mk_regs = detector(
            static_cast<cv::Mat_<std::uint8_t>&>(img), 
            aruco_ids_in_image
        );

        // prepare um domain anchors
        std::vector<cv::Point2d> px_pos;
        std::vector<cv::Point2d> um_pos;
        for(auto [mid, score, loc] : mk_regs) {
            px_pos.push_back(loc);
            auto mkpid = mk_map.get_sub(mid);
            auto mk_x_um = (mkpid.x * mk_w_d_um) + mk_xi_um + (mk_w_um / 2) - st_p.x;
            auto mk_y_um = (mkpid.y * mk_h_d_um) + mk_yi_um + (mk_h_um / 2) - st_p.y;
            um_pos.emplace_back(mk_x_um, mk_y_um);
        }
       //  auto trans_mat = cv::estimateAffinePartial2D(um_pos, px_pos);
        auto trans_mat = warped_mat::estimate_transform_mat(um_pos, px_pos);

        // probe channel process
        auto [pb_templ, pb_mask] = marker::Loader::from_file_to_img(
            pb_mk_path.string(), 
            cl_h_um * rescaled_um2px_r,
            cl_w_um * rescaled_um2px_r,
            sp_w_um * rescaled_um2px_r
        );
        auto [bias, score] = marker::detection::estimate_bias(pb_img, pb_templ, pb_mask, px_pos, trans_mat);
        auto probe_trans_mat = trans_mat.clone();
        {
            auto _bias = bias;
            typed_mat(probe_trans_mat, [&_bias](auto& mat){
                mat(0, 2) += _bias.x;
                mat(1, 2) += _bias.y;
            });
        }
        chipimgproc::ip_convert(pb_img, CV_32F);
        std::cout << trans_mat << std::endl;
        std::cout << probe_trans_mat << std::endl;
        auto warped_mat = make_warped_mat(
            probe_trans_mat, pb_img, 
            {mk_xi_um, mk_yi_um},
            cl_w_um, cl_h_um,
            cl_wd_um, cl_hd_um,
            cl_wn * cl_wd_um,
            cl_hn * cl_hd_um,
            win_r,
            rescaled_um2px_r,
            cl_wn, cl_hn
        );
        warped_mats.emplace_back(std::move(warped_mat));
    }
    auto multi_warped_mat = make_multi_warped_mat(
        std::move(warped_mats),  std::move(stitch_point),
        {0, 0},
        cl_wd_um, cl_hd_um,
        cl_w_um * ((fov_wd_cl * fov_cols) + mk_wd_cl),
        cl_h_um * ((fov_hd_cl * fov_rows) + mk_hd_cl)
    );
    {
        cv::Mat_<std::uint8_t> first_marker(10, 10);
        for(int i = 0; i < 10; i ++) {
            for(int j = 0; j < 10; j ++) {
                first_marker(i, j) = multi_warped_mat.at_cell(i, j + 162).mean / 64;
            }
        }
        // first_marker = binarize(first_marker);
        first_marker = std::get<1>(threshold(first_marker, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU));
        cv::imwrite("test0.png", first_marker);
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
    {
        cv::Mat_<std::uint8_t> first_marker(10, 10);
        for(int i = 0; i < 10; i ++) {
            for(int j = 0; j < 10; j ++) {
                first_marker(i, j) = multi_warped_mat.at_cell(i, j + 324).mean / 64;
            }
        }
        first_marker = binarize(first_marker);
        cv::imwrite("test1.png", first_marker);
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
    {
        cv::Mat_<std::uint8_t> first_marker(10, 10);
        for(int i = 0; i < 10; i ++) {
            for(int j = 0; j < 10; j ++) {
                auto&& cell = multi_warped_mat.at_cell(i + 162, j + 162);
                first_marker(i, j) = cell.mean / 64;
                std::cout << cell.real_p << std::endl;
            }
        }
        // first_marker = binarize(first_marker);
        first_marker = std::get<1>(threshold(first_marker, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU));
        cv::imwrite("test2.png", first_marker);
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
}
