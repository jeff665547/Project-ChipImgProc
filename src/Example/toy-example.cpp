/// [code]
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include <ChipImgProc/stitch/gridline_based.hpp>
#include <ChipImgProc/margin.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
int main()
{
    /*
     *  +=====================+
     *  | Declare input paths |
     *  +=====================+
     */

    std::string image_folder_path = "./Banff_Images/";      //  The path to the image folder, which contains nine FOVs of raw images (Banff)
    std::string marker_pattern_path = "./Banff_CY3.tsv";    //  The path to the marker pattern file
    
    // Store the folder path that contains FOVs in the image_folder_path variable.
    // Store the .tsv file path to the marker pattern in the marker_pattern_path variable.

    /*
     *  +=========+
     *  | Delcare |
     *  +=========+
     */

    //  Declare the marker detector and image detector.
    chipimgproc::marker::detection::RegMat marker_detector;
    chipimgproc::rotation::MarkerVec<float> theta_detector;

    //  Declare the image rotator and image gridder.
    chipimgproc::rotation::Calibrate image_rotator;
    chipimgproc::gridding::RegMat image_gridder;

    //  For cell margin used in the image feature extraction.
    chipimgproc::Margin<double> margin;

    //  Record the segmentation result after image gridding.
    std::vector<chipimgproc::TiledMat<>> tile_matrixs;

    //  Record the tile-matrix (Figure 8) after feature extracting.
    std::vector<chipimgproc::stat::Mats<double>> segmentation_matrixs;

    /*
     *  +=================================================+
     *  | Set each FOV paths and iterator each FOV images |
     *  +=================================================+
     */

    //  Set FOV image paths
    std::vector<std::string> image_fov_paths = {
        image_folder_path + "/0-0-2.tiff",
        image_folder_path + "/0-1-2.tiff",
        image_folder_path + "/0-2-2.tiff",
        image_folder_path + "/1-0-2.tiff",
        image_folder_path + "/1-1-2.tiff",
        image_folder_path + "/1-2-2.tiff",
        image_folder_path + "/2-0-2.tiff",
        image_folder_path + "/2-1-2.tiff",
        image_folder_path + "/2-2-2.tiff"
    };

    for( std::size_t i = 0; i < image_fov_paths.size(); i++ )
    {
        /*
         *  +========================+
         *  | Image data preparation |
         *  +========================+
         */
        
        //  Read the image.
        cv::Mat_<std::uint16_t> image = cv::imread( image_fov_paths[i], cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );
        //  For more information, see the "Fluorescence Marker Detection" section.


        /*
         *  +==============================+
         *  | Marker detection preparation |
         *  +==============================+
         */

        //  Load marker
        std::ifstream marker_file_in( marker_pattern_path );
        auto [ marker, mask ] = chipimgproc::marker::Loader::from_txt( marker_file_in, std::cout );

        //  Create a marker layout for the fluorescence marker detection.
        //  Attention: The generation of the marker layout varies from the types of chips and SUMMIT readers. 
        //  It depends on the particular specification. For more info, you can go to the "Fluorescence Marker 
        //  Detection" section in this tutorial or refer to "unit_test/ChipImgProc/multi_tiled_mat_test.cpp".
        auto marker_layout = chipimgproc::marker::make_single_pattern_reg_mat_layout(
            marker, 
            mask,   
            4,      
            4,      
            1,      
            3,      
            3, 
            81,     
            81,     
            2.68    
        );
        //  For more information, see the "Fluorescence Marker Detection" section.


        /*
         *  +=====================================+
         *  | Marker detection and image rotation |
         *  +=====================================+
         */

        //  Detect the markers.
        auto marker_regions = marker_detector(
            image,
            marker_layout,
            chipimgproc::MatUnit::PX, 
            0,                          
            std::cout
            );
        //  For more information, see the "Fluorescence Marker Detection" section.

        //  Estimate the image rotation angle (degree).
        auto theta = theta_detector( marker_regions, std::cout );
        //  For more information, see the "Image Rotation Angle Estimation and Calibration" section.

        //  Rotate the image via the estimated rotation angle (Image Calibration).
        image_rotator( image, theta );
        //  For more information, see the "Image Rotation Angle Estimation and Calibration" section.

        //  Since the marker positions have been changed after the image calibration, 
        //  we have to re-detect the marker positions.
        
        //  Re-detecting the marker.
        marker_regions = marker_detector( image, marker_layout, chipimgproc::MatUnit::PX, 0, std::cout );
        //  For more information, see the "Fluorescence Marker Detection" section.

        //  Auto-inference to fill the vacant marker positions.
        marker_regions = chipimgproc::marker::detection::reg_mat_infer(
            marker_regions,
            3,                  
            3,
            image
            );
        //  For more information, see the "Image Rotation Angle Estimation and Calibration" section.


        /*
         *  +================+
         *  | Image gridding |
         *  +================+
         */

        //  Image gridding
        auto grid_line = image_gridder( image, marker_layout, marker_regions, std::cout );
        //  For more information, see the "Image Gridding" section.

        //  Convert the gridding result into a tile-matrix (Figure 8).
        auto tile_matrix = chipimgproc::TiledMat<>::make_from_grid_res( grid_line, image, marker_layout );
        //  For more information, see the "Image Feature Extraction" section.


        /*
         *  +==========================+
         *  | Image feature extracting |
         *  +==========================+
         */
        /// [image-feature-extracting]
        //  Set the parameters for feature extraction. 0.6 is the segmentation rate.
        chipimgproc::margin::Param<> feature_extraction_param { 
            0.6, 
            0.17, 
            &tile_matrix,
            true, 
            nullptr
        };
        //  For more information, see the "Image Feature Extraction" section.

        //  Image Feature Extraction (minCV).
        chipimgproc::margin::Result<double> feature_extraction_result = margin(
            "auto_min_cv", 
            feature_extraction_param
        );
        //  For more information, see the "Image Feature Extraction" section.

        //  Save the gridding result
        tile_matrixs.push_back( tile_matrix );

        //  Save the segmentation result..
        segmentation_matrixs.push_back( feature_extraction_result.stat_mats );
    }

    /*
     *  +======================+
     *  | FOV images stitching |
     *  +======================+
     */
    
    //  Set the stitching positions of top-left marker start point for each FOV.
    //  These parameters should be provided by chip & reader specification.
    //  They vary from the types of chips. This example only show the parameters for Banff.
    std::vector<cv::Point_<int>> stitch_positions({
        {0,   0}, {162,   0}, {324,   0},
        {0, 162}, {162, 162}, {324, 162},
        {0, 324}, {162, 324}, {324, 324}
    });
    //  For more information, see the "Image Stitching" section.

    //  Set the FOV sequential ID, the order is row major.
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {2, 0},
        {0, 1}, {1, 1}, {2, 1},
        {0, 2}, {1, 2}, {2, 2}
    });
    //  For more information, see the "Image Stitching" section.    

    //  Create a matrix with multiple tiles (all FOV) for entire image.
    chipimgproc::MultiTiledMat<double, std::uint16_t> multiple_tiled_matrix(
        tile_matrixs,
        segmentation_matrixs,
        stitch_positions,
        fov_ids
    );
    //  For more information, see the "Image Stitching" section.

    //  Set the stitcher and stitch FOVs inside the multi_tiled_mat object into a real pixel-level image.
    chipimgproc::stitch::GridlineBased stitcher;
    auto stitched_image = stitcher( multiple_tiled_matrix );
    //  For more information, see the "Image Stitching" section.

    /*
     *  +===========+
     *  | Outputing |
     *  +===========+
     */

    //  Create a empty heatmap with openCV
    cv::Mat heatmap;

    //  Convert each tile into heatmap with openCV
    multiple_tiled_matrix.dump().convertTo(
        heatmap,
        CV_16U,
        1 
        );


    //  Outputting heatmap data or stitching image directly (default) may cause a low intensity 
    //  image which looks like a black and invisable image.
    //  To deal with this problem, before saving the image, 
    //  we use the chipimgproc::viewable to adjust the brightness of the image.
    
    //  Output heatmap amd stitched images.
    cv::imwrite( "raw_intensity_heatmap.tiff", chipimgproc::viewable( heatmap ));
    cv::imwrite( "stitched_image.tiff", chipimgproc::viewable( stitched_image.mat() ));

    //  Output the raw intensity
    std::ofstream tsv( "raw_intensity.tsv" );
    tsv << multiple_tiled_matrix.dump();
    tsv.close();
    
    return 0;
}
/// [code]
