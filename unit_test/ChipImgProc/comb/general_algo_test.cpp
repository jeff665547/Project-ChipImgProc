#include <ChipImgProc/comb/general_algo.hpp>
// #include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <Nucleona/app/main.hpp>
#include <ChipImgProc/marker/loader.hpp>

// TEST(comb_gridding, basic_test) {
int nucleona::app::main(int argc, char* argv[]) {
    std::ifstream marker_in(
        ( nucleona::test::data_dir() / "zion_pat.tsv").string()
    );
    chipimgproc::comb::GeneralAlgo<float> gridder;
    gridder.set_logger(std::cout);

    gridder.set_grid_res_viewer([](const cv::Mat& m){
        cv::imwrite("debug_grid_result.tiff", m);
    });
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
    gridder.set_roi_res_viewer([](const cv::Mat& m){
        cv::imwrite("debug_roi_res.tiff", m);
    });
    gridder.set_roi_score_viewer([](const cv::Mat& m, int r, int c){
        cv::imwrite("debug_roi_score_" + std::to_string(c) + "_" + std::to_string(r) + ".tiff", m);
    });
    gridder.set_marker_layout(
        { chipimgproc::marker::Loader::from_txt(marker_in, std::cout) }
    );
    auto test_img_path = nucleona::test::data_dir() / "0-0-2.tiff";
    cv::Mat img = cv::imread(test_img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    cv::imwrite("debug_src.tiff", img);
    chipimgproc::info(std::cout, img);
    gridder(img);
}