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
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_cl;
    auto [mk, mask] = chipimgproc::marker::Loader::from_txt(marker_in, std::cout);
    candi_mk_pats_cl.push_back(mk);
    chipimgproc::marker::TxtToImg txt_to_img;
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px_mask;
    auto [mk_img, mask_img] = txt_to_img(
        mk, mask,
        cell_r_um * um2px_r,
        cell_c_um * um2px_r,
        border_um * um2px_r
    );
    candi_mk_pats_px.push_back(mk_img);
    cv::imwrite("mask_img.tif", mask_img);
    cv::imwrite("mk_img.tif",mk_img);
    candi_mk_pats_px_mask.push_back(mask_img);
    chipimgproc::marker::Layout mk_layout;
    std::uint32_t invl_x_px = std::round(invl_x_cl * (cell_c_um + border_um) * um2px_r); // can get this value from micron to pixel
    std::uint32_t invl_y_px = std::round(invl_y_cl * (cell_r_um + border_um) * um2px_r);
    mk_layout.set_reg_mat_dist(
        rows, cols, {0, 0}, 
        invl_x_cl, invl_y_cl, 
        invl_x_px, invl_y_px
    );
    mk_layout.set_single_mk_pat(
        candi_mk_pats_cl, 
        candi_mk_pats_px,
        candi_mk_pats_px_mask
    );
    return mk_layout;
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