#include "gridder.hpp"
#include <Nucleona/proftool/gprofiler.hpp>
#include <Nucleona/app/cli/gtest.hpp>
// #include <Nucleona/app/main.hpp>

TEST(single_image_general_gridding, basic_test) {
    using FLOAT = float;
    auto gridder = get_zion_gridder(2.68);
    auto test_img_paths = {
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-1-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-1-2.tiff"
    };
    using TiledMatT = typename decltype(gridder)::TiledMatT; 
    using Gridline  = typename decltype(gridder)::Gridline;
    std::vector<TiledMatT>                           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<FLOAT>>      stat_mats_s ;

    int i = 0;
    for(auto p : test_img_paths ) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::imwrite("debug_src.tiff", chipimgproc::viewable(img));
        chipimgproc::info(std::cout, img);
        gridder.set_rot_cali_viewer([i](const auto& img){
            cv::imwrite("rot_cali_res" + std::to_string(i) + ".tiff", img);
        });
        gridder.set_grid_res_viewer([i](const auto& img){
            cv::imwrite("grid_res" + std::to_string(i) + ".tiff", img);
        });
        gridder.set_margin_res_viewer([i](const auto& img){
            cv::imwrite("margin_res" + std::to_string(i) + ".tiff", img);
        });
        gridder.set_marker_seg_viewer([i](const auto& img){
            cv::imwrite("marker_seg" + std::to_string(i) + ".tiff", img);
        });
        gridder.set_grid_res_viewer([&i](const cv::Mat& m){
            cv::imwrite("debug_grid_result_"+ std::to_string(i) +".tiff", m);
        });
        gridder.set_marker_seg_append_viewer([&i](const cv::Mat& m){
            cv::Mat tmp = (m * 8.192) + 8192;
            cv::imwrite("marker_append_" + std::to_string(i) + ".tiff", tmp);
        });
        
        auto [qc, tiled_mat, stat_mats, theta, bg_value] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {74, 0}, {0, 74}, {74, 74}
    });
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {0, 1}, {1, 1}
    });
    chipimgproc::MultiTiledMat<FLOAT, Gridline> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps, fov_ids
    );
    cv::Mat md;
    multi_tiled_mat.dump().convertTo(md, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md));
    chipimgproc::stitch::GridlineBased gl_stitcher;
    auto gl_st_img = gl_stitcher(multi_tiled_mat);
    cv::imwrite("raw_stitch.tiff", gl_st_img.mat());
}
TEST(single_image_general_gridding, hard_case_test) {
    using FLOAT = float;
    auto gridder = get_zion_gridder(2.4145);
    auto test_img_paths = {
        nucleona::test::data_dir() / "202_20180612170327" / "0-0-CY3_1M.tiff",
        nucleona::test::data_dir() / "202_20180612170327" / "0-1-CY3_1M.tiff",
        nucleona::test::data_dir() / "202_20180612170327" / "1-0-CY3_1M.tiff",
        nucleona::test::data_dir() / "202_20180612170327" / "1-1-CY3_1M.tiff"
    };
    using TiledMatT = typename decltype(gridder)::TiledMatT; 
    using Gridline  = typename decltype(gridder)::Gridline;
    std::vector<TiledMatT>                           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<FLOAT>>      stat_mats_s ;

    int i = 0;
    for(auto p : test_img_paths ) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::imwrite("debug_src.tiff", img);
        chipimgproc::info(std::cout, img);
        gridder.set_grid_res_viewer([&i](const cv::Mat& m){
            cv::imwrite("debug_grid_result_"+ std::to_string(i) +".tiff", m);
        });
        auto [qc, tiled_mat, stat_mats, theta, bg_value] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {74, 0}, {0, 74}, {74, 74}
    });
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {0, 1}, {1, 1}
    });

    chipimgproc::MultiTiledMat<FLOAT, Gridline> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps, fov_ids
    );
    cv::Mat md;
    multi_tiled_mat.dump().convertTo(md, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md));
    chipimgproc::stitch::GridlineBased gl_stitcher;
    auto gl_st_img = gl_stitcher(multi_tiled_mat);
    cv::imwrite("raw_stitch.tiff", gl_st_img.mat());
}
TEST(single_image_general_gridding, missing_marker_test) {
    using FLOAT = float;
    auto gridder = get_zion_gridder(2.41);
    auto test_img_paths = {
        nucleona::test::data_dir() / "202_20180612170327" / "0-0-CY3_1M.tiff",
        nucleona::test::data_dir() / "marker_missing_test.tiff",
        nucleona::test::data_dir() / "202_20180612170327" / "1-0-CY3_1M.tiff",
        nucleona::test::data_dir() / "202_20180612170327" / "1-1-CY3_1M.tiff"
    };
    using TiledMatT = typename decltype(gridder)::TiledMatT; 
    using Gridline  = typename decltype(gridder)::Gridline;
    std::vector<TiledMatT>                           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<FLOAT>>      stat_mats_s ;

    int i = 0;
    for(auto p : test_img_paths ) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::imwrite("debug_src.tiff", img);
        chipimgproc::info(std::cout, img);
        gridder.set_grid_res_viewer([&i](const cv::Mat& m){
            cv::imwrite("debug_grid_result_"+ std::to_string(i) +".tiff", m);
        });
        auto [qc, tiled_mat, stat_mats, theta, bg_value] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {74, 0}, {0, 74}, {74, 74}
    });

    chipimgproc::MultiTiledMat<FLOAT, Gridline> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps
    );
    cv::Mat md;
    multi_tiled_mat.dump().convertTo(md, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md));
}
/// [usage]
TEST(single_image_general_gridding, banff_test) {
    nucleona::proftool::GProfiler gprofiler("banff_general.prof");
    using FLOAT = float;
    auto gridder = get_banff_gridder("banff_rc/pat_CY3.tsv", 2.68);
    auto test_img_paths = {
        nucleona::test::data_dir() / "banff_test" / "0-0-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "0-1-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "0-2-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "1-0-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "1-1-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "1-2-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "2-0-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "2-1-2.tiff",
        nucleona::test::data_dir() / "banff_test" / "2-2-2.tiff"
    };
    using TiledMatT = typename decltype(gridder)::TiledMatT; 
    using Gridline  = typename decltype(gridder)::Gridline;
    std::vector<TiledMatT>                           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<FLOAT>>      stat_mats_s ;

    int i = 0;
    for(auto p : test_img_paths ) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::imwrite("debug_src.tiff", img);
        chipimgproc::info(std::cout, img);
        gridder.set_marker_seg_viewer([i](const cv::Mat& m ){
            cv::imwrite("debug_marker_seg_result_"+ std::to_string(i) +".tiff", m);
        });
        gridder.set_grid_res_viewer([&i](const cv::Mat& m){
            cv::imwrite("debug_grid_result_"+ std::to_string(i) +".tiff", m);
        });
        auto [qc, tiled_mat, stat_mats, theta, bg_value] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}  , {162, 0}   , {324, 0}, 
        {0, 162}, {162, 162} , {324, 162},
        {0, 324}, {162, 324} , {324, 324},
    });

    chipimgproc::MultiTiledMat<FLOAT, Gridline> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps
    );
    cv::Mat md;
    multi_tiled_mat.dump().convertTo(md, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md));
}
/// [usage]
TEST(single_image_general_gridding, banff_grid_hard_test) {
    using FLOAT = float;
    auto gridder = get_banff_gridder("banff_rc/pat_CY5.tsv", 2.68);
    auto test_img_paths ={
        nucleona::test::data_dir() / "banff_CY5_grid_hard.tiff"
    };
    using TiledMatT = typename decltype(gridder)::TiledMatT; 
    using Gridline  = typename decltype(gridder)::Gridline;
    std::vector<TiledMatT>                           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<FLOAT>>      stat_mats_s ;

    int i = 0;
    for(auto p : test_img_paths ) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::imwrite("debug_src.tiff", img);
        chipimgproc::info(std::cout, img);
        gridder.set_marker_seg_viewer([i](const cv::Mat& m ){
            cv::imwrite("debug_marker_seg_result_"+ std::to_string(i) +".tiff", m);
        });
        gridder.set_grid_res_viewer([&i](const cv::Mat& m){
            cv::imwrite("debug_grid_result_"+ std::to_string(i) +".tiff", m);
        });
        auto [qc, tiled_mat, stat_mats, theta, bg_value] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
}

TEST(single_image_general_gridding, yz01_test) {
    using FLOAT = float;
    auto gridder = get_yz01_gridder("yz01_rc/pat_AM1.tsv", 2.4145);
    std::vector<boost::filesystem::path> test_img_paths;
    for(int i = 0; i < 7; i ++) {
        for(int j = 0; j < 7; j ++) {
            test_img_paths.push_back(
                nucleona::test::data_dir() / "yz01_test" / 
                    (std::to_string(i) + "-" + std::to_string(j) +"-CY3.tiff")
            );
        }
    }
    using TiledMatT = typename decltype(gridder)::TiledMatT; 
    using Gridline  = typename decltype(gridder)::Gridline;
    std::vector<TiledMatT>                           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<FLOAT>>      stat_mats_s ;

    int i = 0;
    for(auto p : test_img_paths ) {
        cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
        cv::imwrite("debug_src.tiff", img);
        chipimgproc::info(std::cout, img);
        gridder.set_marker_seg_viewer([i](const cv::Mat& m ){
            cv::imwrite("debug_marker_seg_result_"+ std::to_string(i) +".tiff", m);
        });
        gridder.set_grid_res_viewer([&i](const cv::Mat& m){
            cv::imwrite("debug_grid_result_"+ std::to_string(i) +".tiff", m);
        });
        auto [qc, tiled_mat, stat_mats, theta, bg_value] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
    int w_d = 202;
    int h_d = 202;
    
    std::vector<cv::Point_<int>> st_ps;
    for(int y = 0; y < 7; y ++) {
        for(int x = 0; x < 7; x ++ ) {
            st_ps.push_back(cv::Point_<int>(x * w_d, y * h_d));
        }
    }

    chipimgproc::MultiTiledMat<FLOAT, Gridline> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps
    );
    cv::Mat md;
    multi_tiled_mat.dump().convertTo(md, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md));
}
