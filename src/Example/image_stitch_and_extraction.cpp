/// [include]
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
#include <boost/program_options.hpp>
/// [include]

/// [code]
/*
 *  This example code will help you to build the application to:
 *      1. Marker detecting
 *      2. Image rotating
 *      3. Image gridding
 *      4. Image feature extracting
 *      5. FOV images stitching
 *
 *  Input:
 *          Folder path which contains FOV
 * 
 *          TSV file of marker pattern
 *          
 *  output:
 *          Rotated image with gridded-line
 * 
 *  Example:
 *          ./Example-aruco_detection
 *              -f banff_test
 *              -m banff_rc/pat_CY3.tsv
 */

int main( int argc, char** argv )
{
    /*
     *  +=========================+
     *  | Declare program options |
     *  +=========================+
     */

    std::string image_folder_path;      //  The path of image folder, which contains nine FOV of raw images
    std::string marker_pattern_path;    //  The path of marker pattern

    boost::program_options::variables_map op;
    boost::program_options::options_description options( "Options" );

    options.add_options()( "help,h" , "Print this help messages" )
        ( "folder,f", boost::program_options::value< std::string >( &image_folder_path   )->required(),"Folder path which contains FOV"  )
        ( "marker,m", boost::program_options::value< std::string >( &marker_pattern_path )->required(),"Tsv file path of marker pattern" )
        ;
    try {
        boost::program_options::store( boost::program_options::parse_command_line( argc, argv, options ), op );
        if( op.count( "help" )) {
            std::cout << "\n" << options << "\n";
            exit(0);
        }
        boost::program_options::notify( op );
    }
    catch( boost::program_options::error& error ) {
        std::cerr << "\nERROR: " << error.what() << "\n" << options << "\n";
        exit(1);
    }

    /*
     *  +=========+
     *  | Delcare |
     *  +=========+
     */

    //  Declare the marker detector and image detector
    chipimgproc::marker::detection::RegMat marker_detector;
    chipimgproc::rotation::MarkerVec<float> theta_detector;

    //  Declare the image rotator and image gridder
    chipimgproc::rotation::Calibrate image_rotator;
    chipimgproc::gridding::RegMat image_gridder;

    //  For cell margin
    chipimgproc::Margin<double> margin;

    //  Record the segmentation result after image gridding
    std::vector<chipimgproc::TiledMat<>> tile_matrixs;

    //  Record the tile-matrix after feature extracting
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

        cv::Mat_<std::uint16_t> image = cv::imread( image_fov_paths[i], cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );

        /*
         *  By default, OpenCV read the image with 8 bits RGB color from cv::imread
         *  We don't want OpenCV reduce our raw chip image to the depth with 8 bits only
         *  So we give the parameter and set to:
         * 
         *      "cv::IMREAD_ANYCOLOR" for RGB color
         *      "cv::IMREAD_ANYDEPTH" for depth with our raw image (16 bits)
         */

        /*
         *  +==============================+
         *  | Marker detection preparation |
         *  +==============================+
         */

        //  Load marker
        std::ifstream marker_file_in( marker_pattern_path );
        auto [ marker, mask ] = chipimgproc::marker::Loader::from_txt( marker_file_in, std::cout );

        auto marker_layout = chipimgproc::marker::make_single_pattern_reg_mat_layout(
            marker, //  Marker
            mask,   //  Mask
            4,      //  Row µm of cell
            4,      //  Column µm of cell
            1,      //  Spacing µm of cell
            3,      //  Row number of markers
            3,      //  Column number of markers
            81,     //  Spacing x of marker
            81,     //  Spacing y of marker
            2.68    //  µm to pixel
        );

        /*
         *
         *  Marker:
         *      Marker pattern loaded from tsv marker pattern file
         * 
         *  Mask:
         *      Marker mask pattern loaded from tsv marker pattern file which set the ignore region for marker detection
         * 
         *  Row µm of cell:
         *      µm in the row for one cell (a probe)
         * 
         *  Column µm of cell:
         *      µm in the Column for one cell (a probe)
         * 
         *  Spacing µm of cell:
         *      µm between cell and cell (a prboe and a probe)
         * 
         *  Row number of markers:
         *      Number of markers ine one Row
         * 
         *  Column number of markers:
         *      Number of markers ine one Column
         * 
         *  Spacing x of marker:
         *      The spacing (cell counts) between first top-left cell (probe) of markers in X axis
         * 
         *  Spacing y of marker:
         *      The spacing (cell counts) between first top-left cell (probe) of markers in Y axis
         * 
         *  µm to pixel:
         *      A magic number which is dependent with the reader (SUMMIT), this parameter is using to convter the µm to pixel
         *      
         * 
         *  Parameter for different chips:
         *                                  Zion    Banff    Yz01
         *               Row µm of cell:    9       4       3
         *            Column µm of cell:    9       4       3
         *           Spacing µm of cell:    2       1       1
         *         Row number of marker:    3       3       3
         *      Column number of marker:    3       3       3
         *          Spacing x of marker:    37      81      101
         *          Spacing y of marker:    37      81      101
         * 
         *                                  DVT-1   DVT-2   DVT-3
         *                  µm to pixel:    2.68    2.68    2.4145
         *                                         (2.4145)
         * 
         *  Marker pattern:
         *      Zion:
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . . X X . . X X . .
         *                          . . X X . . X X . .
         *                          . . . . X X . . . .
         *                          . . . . X X . . . .
         *                          . . X X . . X X . .
         *                          . . X X . . X X . .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *      Banff Cy3 (AM1):
         *                          . . . . . . . . . .
         *                          . X X X . . X X X .
         *                          . X . . . . . . X .
         *                          . X . . . . . . X .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . X . . . . . . X .
         *                          . X . . . . . . X .
         *                          . X X X . . X X X .
         *                          . . . . . . . . . .
         *      Banff Cy5 (AM3):
         *                          . . . . . . . . . .
         *                          . . . . X X . . . .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . X . . . . . . X .
         *                          . X . . . . . . X .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . . . . X X . . . .
         *                          . . . . . . . . . .
         *      Yz01 Cy3 (AM1):
         *                          . . . . . . . . . .
         *                          . X X X . . X X X .
         *                          . X . . . . . . X .
         *                          . X . . . . . . X .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . X . . . . . . X .
         *                          . X . . . . . . X .
         *                          . X X X . . X X X .
         *                          . . . . . . . . . .
         *      Yz01 Cy5 (AM3):
         *                          . . . . . . . . . .
         *                          . . . . X X . . . .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . X . . . . . . X .
         *                          . X . . . . . . X .
         *                          . . . . . . . . . .
         *                          . . . . . . . . . .
         *                          . . . . X X . . . .
         *                          . . . . . . . . . .
         * 
         */

        /*
         *  +=====================================+
         *  | Marker detection and image rotation |
         *  +=====================================+
         */

        //  Detecting marker
        auto marker_regioins = marker_detector(
            image,
            marker_layout,
            chipimgproc::MatUnit::PX,   //  Marker detecte with pixel unit (PX) or cell unit (CELL) 
            0,                          //  Default mode to use perfect marker pattern
            std::cout
            );

        //  Detecting image rotation theta (degree)
        auto theta = theta_detector( marker_regioins, std::cout );

        //  Rotating the image via detected theta (degree)
        image_rotator( image, theta );

        //  Re-detecting the marker
        marker_regioins = marker_detector( image, marker_layout, chipimgproc::MatUnit::PX, 0, std::cout );

        //  Auto-inference to fill the vacancy marker positions
        marker_regioins = chipimgproc::marker::detection::reg_mat_infer(
            marker_regioins,
            3,                  //  Number of markers ine one Row
            3,                  //  Number of markers ine one Column
            image
            );

        /*
         *  +================+
         *  | Image gridding |
         *  +================+
         */

        //  Image gridding and output the result via lambda expression
        auto grid_line = image_gridder( image, marker_layout, marker_regioins, std::cout );

        //  Convert the gridding result into a tile-matrix
        auto tile_matrix = chipimgproc::TiledMat<>::make_from_grid_res( grid_line, image, marker_layout );

        /*
         *  +==========================+
         *  | Image feature extracting |
         *  +==========================+
         */

        //  Set the parameters for feature extraction
        chipimgproc::margin::Param<> feature_extraction_param { 
            0.6,            //  Segmentation rate
            &tile_matrix,
            true,           //  Tile replacement
            nullptr
        };

        /*
         *  Segmentation rate:
         *      The % of margin segmentation from outside to inside, only use in "mid_seg" mode
         *      Default rate is 0.6
         * 
         *  Tile replacement:
         *      Is segmented tile result replace the tile-matrix or not
         */

        //  Feature extracting
        chipimgproc::margin::Result<double> feature_extraction_result = margin(
            "auto_min_cv",              //  Feature extracting mode
            feature_extraction_param
        );

        /*
         *  Feature extracting mode:
         *      Set feature extracting mode, here are two modes:
         * 
         *          "mid_seg":
         *              The default mode for Feature extraction, use only segmentation rate and reducing the tile from outside to inside
         * 
         *          "auto_min_cv":
         *              Ignore the segmentation rate and find the minimum CV of this tile automatically
         */

        //  Save the gridding result
        tile_matrixs.push_back( tile_matrix );

        //  Save the segmentation result
        segmentation_matrixs.push_back( feature_extraction_result.stat_mats );
    }

    /*
     *  +======================+
     *  | FOV images stitching |
     *  +======================+
     */
    
    //  Set stitch positions of top-left marker start point for each FOV
    std::vector<cv::Point_<int>> stitch_positions({
        {0,   0}, {162,   0}, {324,   0},
        {0, 162}, {162, 162}, {324, 162},
        {0, 324}, {162, 324}, {324, 324}
    });

    /*
     *  The point (x,y) of stitch positions are base on the spacing of the markers, for example:
     * 
     *      $ Spacing (x,y) of markers for Banff are both "81"
     * 
     *      $ Banff images have nine FOV, and each FOV contain three markers at both X-axis and Y-axis
     * 
     *      $ The overlapping between two FOV of Banff images are both one column and one row of markers
     * 
     *      $ Example of entire Banff image:
     * 
     *               0    81   162  243  324  405  486
     *              +--------------------------------+
     *            0 |[]   []   []   []   []   []   []|      [] are presenting as markers
     *              |                                |
     *           81 |[]   []   []   []   []   []   []|
     *              |                                |
     *          162 |[]   []   []   []   []   []   []|
     *              |                                |
     *          243 |[]   []   []   []   []   []   []|
     *              |                                |
     *          324 |[]   []   []   []   []   []   []|
     *              |                                |
     *          405 |[]   []   []   []   []   []   []|
     *              |                                |
     *          486 |[]   []   []   []   []   []   []|
     *              +--------------------------------+
     *                      Entire Banff Image
     * 
     *      $ Example of overlapping and stitching at three FOVs:
     * 
     *         StitchingPoint       StitchingPoint      StitchingPoint
     *               |                    |                   |
     *               |  OverlappingMarker | OverlappingMarker |  OverlappingMarker
     *               |          |         |         |         |          |
     *               V          V         V         V         V          V
     *               0    81   162       162  243  324       324   405  486
     *              +------------+      +------------+       +------------+
     *            0 |[]   []   []|      |[]   []   []|       |[]   []   []|
     *              |            |      |            |       |            |
     *           81 |[]   []   []|      |[]   []   []|       |[]   []   []|
     *              |            |      |            |       |            |
     *          162 |[]   []   []|      |[]   []   []|       |[]   []   []|
     *              +------------+      +------------+       +------------+
     *                 FOV(0,0)            FOV(1,0)             FOV(2,0)
     */

    //  Set the FOV sequencial ID, the order is row major
    std::vector<cv::Point> fov_ids({
        {0, 0}, {1, 0}, {2, 0},
        {0, 1}, {1, 1}, {2, 1},
        {0, 2}, {1, 2}, {2, 2}
    });

    //  Create a matrix with multiple tiles (all FOV) for entire image
    chipimgproc::MultiTiledMat<double, std::uint16_t> multiple_tiled_matrix(
        tile_matrixs,
        segmentation_matrixs,
        stitch_positions,
        fov_ids
    );

    //  Set stitcher and stitch FOVs in real pixel level of image
    chipimgproc::stitch::GridlineBased stitcher;
    auto stitched_image = stitcher( multiple_tiled_matrix );

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
        CV_16U,     //  Convert float to 16uint in openCV
        1           //  Scaling
        );

    //  Output heatmap amd stitched images
    cv::imwrite( "raw_intensity_heatmap.tiff", chipimgproc::viewable( heatmap ));
    cv::imwrite( "stitched_image.tiff", chipimgproc::viewable( stitched_image.mat() ));

    /*
     *  Output the heatmap data or stitched image by default may generate a low-value image, which is not viewable
     *  chipimgproc::viewable() will help to generate viewable image
     */

    //  Output the raw intensity
    std::ofstream tsv( "raw_intensity.tsv" );
    tsv << multiple_tiled_matrix.dump();
    tsv.close();
    
    return 0;
}
/// [code]