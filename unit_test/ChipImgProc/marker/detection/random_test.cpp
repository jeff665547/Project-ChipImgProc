#include <ChipImgProc/marker/detection/random.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/view.hpp>

using namespace chipimgproc;

TEST(random_test, basic_test) {
    int cell_r_um   = 4;
    int cell_c_um   = 4;
    int space_um    = 1;
    const double um2px_r = 2.4145;
    const double rescale = um2px_r * 2;
    auto mk_pat_path = nucleona::test::data_dir() / "banff_rc" / "pat_CY5.tsv";
    auto img_path = nucleona::test::data_dir() / "aruco-green-pair" / "0-1-CY3.tiff"; // in focus
    auto random_matcher = marker::detection::make_random(
        mk_pat_path.string(),
        cell_r_um * um2px_r, 
        cell_c_um * um2px_r,
        space_um  * um2px_r, 
        2, 16383.0, 9, 
        50  * um2px_r
    );
    auto img = cv::imread(img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
    auto match_res = random_matcher(img);
    std::vector<cv::Point> mk_pts;
    for(auto&& [mkid, score, mk_pt] : match_res) {
        std::cout << mkid << std::endl;
        std::cout << mk_pt << std::endl;
        mk_pts.emplace_back(
            std::round(mk_pt.x),
            std::round(mk_pt.y)
        );
    }
    auto test_view = marker::view(img, mk_pts);
    cv::imwrite("test.png", test_view);
    // unable to test result
}