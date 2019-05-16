#include <ChipImgProc/algo/um2px_auto_scale.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include "../make_layout.hpp"
TEST(um2px_auto_scale, basic_test_zion) {
    auto mk_pat_path = nucleona::test::data_dir() / "zion_pat_am3.tsv";
    auto image_path = nucleona::test::data_dir() / "zion_am3_test.tiff";
    std::ifstream mk_pat_if(mk_pat_path.string());
    auto [mk_img, mk_mask_img] = chipimgproc::marker::Loader::from_txt(
        mk_pat_if, std::cout
    );
    auto img = chipimgproc::imread(image_path);
    chipimgproc::algo::Um2PxAutoScale auto_scaler(
        img, 9, 9, 2
    );
    chipimgproc::marker::Layout mk_layout = make_zion_layout2("zion_pat_am3.tsv", 2.4145);
    auto [best_um2px_r, score_mat] = auto_scaler.linear_steps(
        mk_layout, 2.4145, 0.002, 5, {}, std::cout 
    );
    std::cout << best_um2px_r << std::endl;
    EXPECT_LT(best_um2px_r, 2.4145 + 0.05);
    EXPECT_GT(best_um2px_r, 2.4145 - 0.05);
}

TEST(um2px_auto_scale, basic_test_banff) {
    auto mk_pat_path = nucleona::test::data_dir() / "banff_rc" / "pat_CY5.tsv";
    auto image_path = nucleona::test::data_dir() / "banff_AM3_missing_marker.tiff";
    std::ifstream mk_pat_if(mk_pat_path.string());
    auto [mk_img, mk_mask_img] = chipimgproc::marker::Loader::from_txt(
        mk_pat_if, std::cout
    );
    auto img = chipimgproc::imread(image_path);
    chipimgproc::algo::Um2PxAutoScale auto_scaler(
        img, 4, 4, 1
    );
    chipimgproc::marker::Layout mk_layout = make_banff_layout("banff_rc/pat_CY5.tsv", 2.4145);
    auto [best_um2px_r, score_mat] = auto_scaler.linear_steps( 
        mk_layout, 2.4145, 0.002, 5, {}, std::cout );
    std::cout << best_um2px_r << std::endl;
    EXPECT_LT(best_um2px_r, 2.4145 + 0.05);
    EXPECT_GT(best_um2px_r, 2.4145 - 0.05);
}