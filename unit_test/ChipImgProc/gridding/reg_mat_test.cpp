#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
auto pat_img(const std::string& id) {
    auto zion_pat_px_path = nucleona::test::data_dir() / id;
    auto mk_px_ = cv::imread(zion_pat_px_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    chipimgproc::info(std::cout, mk_px_);
    cv::Mat_<std::uint8_t> mk_px;
    chipimgproc::info(std::cout, mk_px);
    cv::extractChannel(mk_px_, mk_px, mk_px_.channels() - 1);
    return mk_px;

}
TEST(reg_mat_layout, operator_call_test) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;
    chipimgproc::rotation::Calibrate rot_cali;
    chipimgproc::gridding::RegMat gridding;

    // prepare image
    auto img_path = 
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff";
    cv::Mat_<std::uint16_t>img = cv::imread(
        img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
    );

    // prepare marker
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / "zion_pat.tsv").string()
    );
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_cl;
    candi_mk_pats_cl.push_back(
        chipimgproc::marker::Loader::from_txt(marker_in, std::cout)
    );
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
    candi_mk_pats_px.push_back(pat_img("zion_pat_295px.tif"));
    chipimgproc::marker::Layout mk_layout;
    mk_layout.set_reg_mat_dist(
        3,3, {0, 0}, 37, 37, 1091, 1091
    );
    mk_layout.set_single_mk_pat(
        candi_mk_pats_cl, candi_mk_pats_px
    );
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    rot_cali(img, theta);
    mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, std::cout);
    auto gl_res = gridding(img, mk_layout, mk_regs, std::cout, [](const auto& m){
        cv:imwrite("debug_gridding.tiff", m);
    });
}
TEST(reg_mat_layout, hard_case) {
    chipimgproc::marker::detection::RegMat reg_mat;
    chipimgproc::rotation::MarkerVec<float> marker_fit;
    chipimgproc::rotation::Calibrate rot_cali;
    chipimgproc::gridding::RegMat gridding;

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
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_cl;
    candi_mk_pats_cl.push_back(
        chipimgproc::marker::Loader::from_txt(marker_in, std::cout)
    );
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
    candi_mk_pats_px.push_back(pat_img("zion_pat_265px.tif"));
    chipimgproc::marker::Layout mk_layout;
    mk_layout.set_reg_mat_dist(
        3,3, {0, 0}, 37, 37, 1091, 1091
    );
    mk_layout.set_single_mk_pat(
        candi_mk_pats_cl, candi_mk_pats_px
    );
    auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, std::cout);
    auto theta = marker_fit(mk_regs, std::cout);
    rot_cali(img, theta);
    mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, std::cout);
    auto gl_res = gridding(img, mk_layout, mk_regs, std::cout, [](const auto& m){
        cv:imwrite("debug_gridding.tiff", m);
    });
}