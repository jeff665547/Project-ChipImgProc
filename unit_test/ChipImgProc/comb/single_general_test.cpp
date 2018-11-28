#include <ChipImgProc/comb/single_general.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/stitch/gridline_based.hpp>
#include <ChipImgProc/marker/txt_to_img.hpp>
auto pat_img(const std::string& path) {
    auto mk_px_ = cv::imread(path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    chipimgproc::info(std::cout, mk_px_);
    cv::Mat_<std::uint8_t> mk_px;
    cv::extractChannel(mk_px_, mk_px, mk_px_.channels() - 1);
    chipimgproc::info(std::cout, mk_px);
    cv::imwrite("debug_marker.tiff", mk_px);
    return mk_px;
}
auto get_gridder(
    const std::string& patname, 
    float cell_r_px,
    float cell_c_px,
    float border_px,
    int rows, 
    int cols,
    std::uint32_t invl_x_cl, 
    std::uint32_t invl_y_cl,
    std::uint32_t invl_x_px, // can get this value from micron to pixel
    std::uint32_t invl_y_px,
    float um2px_r
) {
    chipimgproc::marker::TxtToImg txt_to_img;
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / patname).string()
    );
    chipimgproc::comb::SingleGeneral<> gridder;
    gridder.disable_background_fix(true);
    gridder.set_logger(std::cout);
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_cl;
    auto [mk, mask] = chipimgproc::marker::Loader::from_txt(marker_in, std::cout);
    candi_mk_pats_cl.push_back(mk);

    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px_mask;
    auto [mk_img, mask_img] = txt_to_img(
        mk, mask,
        cell_r_px * um2px_r,
        cell_c_px * um2px_r,
        border_px * um2px_r
    );
    candi_mk_pats_px.push_back(mk_img);
    cv::imwrite("mask_img.tif", mask_img);
    cv::imwrite("mk_img.tif",mk_img);
    candi_mk_pats_px_mask.push_back(mask_img);


    gridder.set_margin_method("auto_min_cv");
    gridder.set_marker_layout(
        candi_mk_pats_cl, 
        candi_mk_pats_px,
        candi_mk_pats_px_mask,
        rows, cols, 
        invl_x_cl, invl_y_cl, 
        invl_x_px, invl_y_px
    );
    return gridder;

}
auto get_zion_gridder(float um2px_r) {
    auto gridder = get_gridder(
        "zion_pat.tsv",
        9, 9, 2,
        3, 3, 
        37, 37,
        1091, 1091,
        um2px_r
    );
    return gridder;
}
auto get_banff_gridder(const std::string& pat_name, float um2px_r) {
    auto gridder = get_gridder(pat_name, 
        4, 4, 1, 3, 3, 81, 81, 1085, 1085, um2px_r
    );
    return gridder;
}
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
        auto [qc, tiled_mat, stat_mats, theta] = gridder(img, p.replace_extension("").string());
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
    auto gridder = get_zion_gridder(2.41);
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
        auto [qc, tiled_mat, stat_mats, theta] = gridder(img, p.replace_extension("").string());
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
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md, 0.02, 0.02));
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
        auto [qc, tiled_mat, stat_mats, theta] = gridder(img, p.replace_extension("").string());
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
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md, 0.02, 0.02));
}
/// [usage]
TEST(single_image_general_gridding, banff_test) {
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
        auto [qc, tiled_mat, stat_mats, theta] = gridder(img, p.replace_extension("").string());
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
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md, 0.02, 0.02));
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
        auto [qc, tiled_mat, stat_mats, theta] = gridder(img, p.replace_extension("").string());
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(stat_mats);
        i ++;
    }
}
