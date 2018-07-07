#include <ChipImgProc/comb/multi_general.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
// #include <Nucleona/app/main.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/stat/mats.hpp>

TEST(multi_image_general_gridding, basic_test) {
// int nucleona::app::main(int argc, char* argv[]) {
    using FLOAT = float;
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / "zion_pat.tsv").string()
    );
    chipimgproc::comb::MultiGeneral<> gridder;
    gridder.set_logger(std::cout);
    gridder.set_rot_cali_viewer([]( const cv::Mat& m ){
        cv::imwrite("debug_rot_cali.tiff", m);
    });
    gridder.set_edges_viewer([]( const cv::Mat& m ){
        cv::imwrite("debug_edge.tiff", m);
    });
    gridder.set_hough_tf_viewer([](const cv::Mat& m){
        cv::imwrite("debug_hough_tf.tiff", m);
    });
    gridder.set_margin_res_viewer([](const cv::Mat& m){
        cv::imwrite("debug_margin.tiff", m);
    });
    gridder.set_roi_bin_viewer([](const cv::Mat& m){
        cv::imwrite("debug_roi_bin.tiff", m);
    });
    gridder.set_roi_score_viewer([](const cv::Mat& m, int r, int c){
        cv::imwrite("debug_roi_score_" + std::to_string(c) + "_" + std::to_string(r) + ".tiff", m);
    });
    std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats;
    auto mk = chipimgproc::marker::Loader::from_txt(marker_in, std::cout);
    candi_mk_pats.push_back(mk);
    gridder.set_marker_layout(
        candi_mk_pats
    );
    std::vector<boost::filesystem::path> test_img_paths ({
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-1-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-1-2.tiff"
    });
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {0, 74}, {74, 0}, {74, 74}
    });
    
    auto multi_tiled_mat = gridder(test_img_paths, st_ps);
    cv::Mat md;
    multi_tiled_mat.dump().convertTo(md, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(md));
}