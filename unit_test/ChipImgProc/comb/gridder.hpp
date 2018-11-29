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
#include "../make_layout.hpp"
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
    T gridder;
    gridder.disable_background_fix(true);
    gridder.set_logger(std::cout);
    gridder.set_margin_method("auto_min_cv");
    gridder.set_marker_layout(make_layout(
        patname, 
        cell_r_px, cell_c_px,
        border_px,
        rows, cols,
        invl_x_cl, invl_y_cl,
        invl_x_px, invl_y_px,
        um2px_r
    ));
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