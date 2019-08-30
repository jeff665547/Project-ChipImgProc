#pragma once
#include <vector>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/txt_to_img.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>

auto make_layout(
    const std::string& patname, 
    float cell_r_um,
    float cell_c_um,
    float border_um,
    int rows, 
    int cols,
    std::uint32_t invl_x_cl, 
    std::uint32_t invl_y_cl,
    float um2px_r
) {
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / patname).string()
    );
    auto [mk, mask] = chipimgproc::marker::Loader::from_txt(marker_in, std::cout);
    return chipimgproc::marker::make_single_pattern_reg_mat_layout(
        mk, mask, cell_r_um, cell_c_um, border_um, rows, cols,
        invl_x_cl, invl_y_cl, um2px_r
    );
}
auto make_zion_layout(float um2px_r) {
    return make_layout(
        "zion_pat.tsv",
        9, 9, 2,
        3, 3, 
        37, 37,
        um2px_r
    );
}
auto make_zion_layout2(const std::string& pat_name, float um2px_r) {
    return make_layout(
        pat_name,
        9, 9, 2,
        3, 3, 
        37, 37,
        um2px_r
    );
}
auto make_banff_layout(const std::string& pat_name, float um2px_r) {
    return make_layout(
        pat_name, 
        4, 4, 1, 
        3, 3, 
        81, 81, 
        um2px_r
    );
}
auto make_yz01_layout(const std::string& pat_name, float um2px_r) {
    return make_layout(
        pat_name, 
        3, 3, 1, 
        3, 3, 
        101, 101, 
        um2px_r
    );
}