#include <ChipImgProc/algo/um2px_auto_scale.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include "../make_layout.hpp"
TEST(um2px_auto_scale, basic_test) {
    auto mk_pat_path = nucleona::test::data_dir() / "zion_pat_am3.tsv";
    auto image_path = nucleona::test::data_dir() / "zion_am3_test.tiff";
    std::ifstream mk_pat_if(mk_pat_path.string());
    auto [mk_img, mk_mask_img] = chipimgproc::marker::Loader::from_txt(
        mk_pat_if, std::cout
    );
    auto img = chipimgproc::imread(image_path);
    chipimgproc::algo::Um2PxAutoScale auto_scaler(
        img, mk_img, mk_mask_img, 9, 9, 2, 3, 3, 37, 37
    );
    auto best_um2px_r = auto_scaler.linear_steps(2.4145, 0.001, 5, std::cout );
    std::cout << best_um2px_r << std::endl;
}
