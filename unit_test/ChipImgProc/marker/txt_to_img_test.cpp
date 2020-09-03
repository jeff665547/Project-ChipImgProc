#include <ChipImgProc/marker/txt_to_img.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST(txt_to_img_test, basic_test) {
    const double um2px_r = 2.4145;
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / "banff_rc" / "pat_CY3.tsv").string()
    );
    auto [mk, mask] = chipimgproc::marker::Loader::from_txt(marker_in, std::cout);
    auto [templ_img, mask_img] = chipimgproc::marker::txt_to_img(mk, mask, 4 * um2px_r, 4 * um2px_r, 1 * um2px_r, std::cout);
    cv::imwrite("templ.png", templ_img);
    cv::imwrite("mask.png", mask_img);
    std::cout << templ_img.size() << '\n';
    std::cout << mask_img.size() << '\n';

}