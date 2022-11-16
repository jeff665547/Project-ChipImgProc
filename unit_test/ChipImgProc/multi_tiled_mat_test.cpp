/// [usage]
#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/marker/detection/reg_mat.hpp>
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
    /*
        This is a example of using chipimgproc::MultiTiledMat and also,
        it is a example processing the raw FOV images to the final heatmap.
    */
    /*
        First, we create the tools we need, the usage of them can see the API document
    */
    chipimgproc::marker::detection::RegMat      reg_mat     ;  // for marker detection
    chipimgproc::rotation::MarkerVec<double>    marker_fit  ;  // for rotation inference
    chipimgproc::rotation::Calibrate            rot_cali    ;  // for rotation calibration
    chipimgproc::gridding::RegMat               gridding    ;  // for image gridding
    chipimgproc::Margin<double>                 margin      ;  // for cell margin

    /*
        Create FOV process result storage.
        We store gridding result(tiled_mats) and margin result(stat_mats_s)
        seperately.
    */
    std::vector<chipimgproc::TiledMat<>>           tiled_mats  ; // 
    std::vector<chipimgproc::stat::Mats<double>>   stat_mats_s ;

    /*
        Set input FOV images path.
    */
    std::vector<boost::filesystem::path> test_img_paths ({
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "0-1-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-0-2.tiff",
        nucleona::test::data_dir() / "C018_2017_11_30_18_14_23" / "1-1-2.tiff"
    });


    for(std::size_t i = 0; i < test_img_paths.size(); i ++ ) {
        // Read FOV image
        auto& img_path = test_img_paths[i];
        std::cout << img_path.string() << std::endl;
        cv::Mat_<std::uint16_t>img = cv::imread(
            img_path.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH
        );

        // Create ZION marker layout and set the micron to pixel rate to 2.68
        // In real application, the marker layout generation is depend on chip and reader specification, 
        // which is application level staff and the ChipImgProc would not collect such parameters.
        // In example code, we just hard code everything. 
        // The hard code marker layout generation process can be found in unit_test/ChipImgProc/multi_tiled_mat_test.cpp
        auto mk_layout = make_zion_layout(2.68);

        // Detect markers
        auto mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

        // Evaluate the rotation degree
        auto theta = marker_fit(mk_regs, std::cout);

        // Rotation calibrate image
        rot_cali(img, theta);

        // Re-detect the markers, 
        // because after the image rotation, the marker position is changed.
        mk_regs = reg_mat(img, mk_layout, chipimgproc::MatUnit::PX, 0, std::cout);

        // Inference the missing marker and standardize the marker position.
        mk_regs = chipimgproc::marker::detection::reg_mat_infer(mk_regs, 0, 0, img);

        // Do the gridding, and pass the debug viewer into algorithm. 
        auto gl_res = gridding(img, mk_layout, mk_regs, std::cout);

        // Make tiled matrix from gridding result.
        auto tiled_mat = chipimgproc::TiledMat<>::make_from_grid_res(gl_res, img, mk_layout);

        // Prepare margin parameter, we set segment rate=0.6 and 
        // replace the tile rectangle after margin.
        chipimgproc::margin::Param<> margin_param { 
            0.6, 0.17, &tiled_mat, true, nullptr
        };

        // Call margin use auto minimum CV search algorithm.
        chipimgproc::margin::Result<double> margin_res = margin(
            "auto_min_cv", margin_param
        );

        // Save the gridding result
        tiled_mats.push_back(tiled_mat);

        // Save the margin result
        stat_mats_s.push_back(margin_res.stat_mats);
    }
    
    // Set the logical cell level stitch position.
    // This parameter should provide by chip & reader specification.
    // In the real application case
    std::vector<cv::Point_<int>> st_ps({
        {0, 0}, {74, 0}, {0, 74}, {74, 74}
    });

    // Set the FOV sequencial ID, the order is row major. 
    // Provide this parameter 
    // can make multi_tiled_mat have ability to access FOV image from FOV ID.
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {0, 1}, {1, 1}
    });

    // Create multiple tiled matrix.
    chipimgproc::MultiTiledMat<double, std::uint16_t> multi_tiled_mat(
        tiled_mats, stat_mats_s, st_ps, fov_ids
    );

    /* 
        Generate results.
        In this example, we generate mean value heatmap and raw image stitching result.
    */

    // Create heatmap storage.
    cv::Mat heatmap;

    // Dump multi_tiled_mat use default cell info accessor (min_cv_mean).
    // The dump result is actually a float point matrix.
    // For image output to tiff format, we have to convert it into integer matrix.
    multi_tiled_mat.dump().convertTo(heatmap, CV_16U, 1);

    // Direct output heatmap data may generate a low value image,
    // which is near all black and unvisable.
    // Therefore, before write image, 
    // we use chipimgproc::viewable to calibrate the brightness of the image
    cv::imwrite("means_dump.tiff", chipimgproc::viewable(heatmap));

    // In heatmap image, a pixel represent a cell, 
    // but we may still need raw image to do some other analysis.
    // Here we stitch the image inside multi_tiled_mat into a real pixel level single image.
    chipimgproc::stitch::GridlineBased gl_stitcher;
    auto gl_st_img = gl_stitcher(multi_tiled_mat);

    // Direct output heatmap data may generate a low value image,
    // which is near all black and unvisable.
    // Therefore, before write image, 
    // we use chipimgproc::viewable to calibrate the brightness of the image
    cv::imwrite("stitch.tiff", chipimgproc::viewable(gl_st_img.mat()));
}
/// [usage]
