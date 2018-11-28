#pragma once
#include <ChipImgProc/comb/single_general.hpp>
#include <ChipImgProc/comb/multi_general.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/stitch/gridline_based.hpp>
#include <ChipImgProc/marker/txt_to_img.hpp>
template<class T>
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
    T gridder;
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
    auto gridder = get_gridder<chipimgproc::comb::SingleGeneral<>>(
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
    auto gridder = get_gridder<chipimgproc::comb::SingleGeneral<>>(
        pat_name, 
        4, 4, 1, 3, 3, 81, 81, 1085, 1085, um2px_r
    );
    return gridder;
}
auto get_zion_multi_gridder(float um2px_r) {
    auto gridder = get_gridder<chipimgproc::comb::MultiGeneral<>>(
        "zion_pat.tsv",
        9, 9, 2,
        3, 3, 
        37, 37,
        1091, 1091,
        um2px_r
    );
    return gridder;
}