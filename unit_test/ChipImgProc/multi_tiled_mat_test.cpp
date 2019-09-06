#include <Nucleona/app/cli/gtest.hpp>
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
#include <ChipImgProc/margin.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
#include <ChipImgProc/stitch/gridline_based.hpp>
#include "./make_layout.hpp"

TEST(multi_tiled_mat, basic_test) {

    chipimgproc::marker::detection::RegMat      reg_mat     ;  // for marker detection
    chipimgproc::rotation::MarkerVec<double>    marker_fit  ;  // for rotation inference
    chipimgproc::rotation::Calibrate            rot_cali    ;  // for rotation calibration
    chipimgproc::gridding::RegMat               gridding    ;  // for image gridding
    chipimgproc::Margin<double>                 margin      ;  // for cell margin

    std::vector<chipimgproc::TiledMat<>>           tiled_mats  ;
    std::vector<chipimgproc::stat::Mats<double>>   stat_mats_s ;

    std::vector<boost::filesystem::path> test_img_paths ({
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-1-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-1-2.tiff"
    });

    for(std::size_t i = 0; i < test_img_paths.size(); i ++ ) {
        auto& img_path = test_img_paths[i];
        cv::Mat_<std::uint16_t>img = cv::imread(
            img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
        );

        // create ZION marker layout and set the micron to pixel rate to 2.68
        auto mk_layout = make_zion_layout(2.68);

        // detect markers
        auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

        // evaluate the rotation degree
        auto theta = marker_fit(mk_regs, std::cout);

        // rotation calibrate image
        rot_cali(img, theta);

        // re-detect the markers, 
        // because after the image rotation, the marker position is changed.
        mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

        // inference the missing marker and standardize the marker position.
        mk_regs = chipimgproc::marker::detection::reg_mat_infer(mk_regs, 0, 0, img);

        // do the gridding, and pass the debug viewer into algorithm. 
        auto gl_res = gridding(img, mk_layout, mk_regs, std::cout);
        auto tiled_mat = chipimgproc::TiledMat<>::make_from_grid_res(gl_res, img, mk_layout);
        chipimgproc::margin::Param<> margin_param { 
            0.6, &tiled_mat, true, nullptr
        };
        chipimgproc::margin::Result<double> margin_res = margin(
            "auto_min_cv", margin_param
        );
        tiled_mats.push_back(tiled_mat);
        stat_mats_s.push_back(margin_res.stat_mats);
    }
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {74, 0}, {0, 74}, {74, 74}
    });
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {0, 1}, {1, 1}
    });
    chipimgproc::MultiTiledMat<double, std::uint16_t> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps, fov_ids
    );
    cv::Mat heatmap;
    multi_tiled_mat.dump().convertTo(heatmap, CV_16U, 1);
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(heatmap));
    chipimgproc::stitch::GridlineBased gl_stitcher;
    auto gl_st_img = gl_stitcher(multi_tiled_mat);
    cv::imwrite("stitch.tiff", chipimgproc::viewable(gl_st_img.mat()));
}