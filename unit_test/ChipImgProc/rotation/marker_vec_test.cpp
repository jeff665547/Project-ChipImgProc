#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>

TEST(reg_mat_layout, operator_call_test) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / "zion_pat.tsv").string()
    );
    auto zion_pat_px_path = nucleona::test::data_dir() / "zion_pat_295px.tif";
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_cl;
    candi_mk_pats_cl.push_back(
        chipimgproc::marker::Loader::from_txt(marker_in, std::cout)
    );
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
    candi_mk_pats_px.push_back(
        cv::imread(zion_pat_px_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH )
    );
    chipimgproc::marker::Layout mk_layout;
    mk_layout.set_reg_mat_dist(
        3,3, {0, 0}, 37, 37, 1091, 1091
    );
    mk_layout.set_single_mk_pat(
        candi_mk_pats_cl, candi_mk_pats_px
    );
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    std::cout << theta << std::endl;
}
TEST(reg_mat_layout, hard_case) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "202_20180612170327" / "0-1-CY3_1M.tiff";
    cv::Mat_<std::uint16_t> img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / "zion_pat.tsv").string()
    );
    auto zion_pat_px_path = nucleona::test::data_dir() / "zion_pat_265px.tif";
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_cl;
    candi_mk_pats_cl.push_back(
        chipimgproc::marker::Loader::from_txt(marker_in, std::cout)
    );
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
    candi_mk_pats_px.push_back(
        cv::imread(zion_pat_px_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH )
    );
    chipimgproc::marker::Layout mk_layout;
    mk_layout.set_reg_mat_dist(
        3,3, {0, 0}, 37, 37, 1091, 1091
    );
    mk_layout.set_single_mk_pat(
        candi_mk_pats_cl, candi_mk_pats_px
    );
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    std::cout << theta << std::endl;
}