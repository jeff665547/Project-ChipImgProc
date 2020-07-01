#include <nlohmann/json.hpp>
#include <ChipImgProc/aruco.hpp>
#include <boost/program_options.hpp>
#include <ChipImgProc/marker/detection/aruco_random.hpp>
#include <iostream>

/*
 *  This example code will help you to build the Image-ArUco detection app. 
 *
 *  Input:
 *          TIFF file of chip image
 *          (The raw image from SUMMIT)
 * 
 *          JSON file of ArUco database
 *          (A json database which stores 250 of ArUco code with 6x6 bits size each)
 * 
 *          Vector of ArUco ID and sort with ArUco positions in the chip image
 *          (Set the relative coordinate of each ArUco code in the chip image)
 * 
 *          TIFF file of marker template frame
 *          TIFF file of marker mask frame
 *          (The marker images for the inside frame pattern and the outside frame border)
 *          
 *  output:
 *          The ArUco ID with position(x,y) on the chip image
 * 
 *  Example:
 *          ./Example-aruco_detection
 *              -i aruco_test_img-0.tiff
 *              -j aruco_db.json
 *              -t aruco_frame_template.tiff
 *              -m aruco_frame_mask.tiff
 */

int main( int argc, char** argv )
{
    /// [data_preparation]

    /*
     *  +=========================+
     *  | Declare program options |
     *  +=========================+
     */

    std::string image_path;                 //  The path of raw chip image
    std::string aruco_database_path;        //  The path of ArUco database

    boost::program_options::variables_map op;
    boost::program_options::options_description options( "Options" );

    options.add_options()( "help,h" , "Print this help messages" )
        ( "image,i"    , boost::program_options::value< std::string >( &image_path                 )->required(),"Image path of raw chip image" )
        ( "json,j"     , boost::program_options::value< std::string >( &aruco_database_path        )->required(),"Json path of ArUco database" )
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

/// [data_preparation]
    /*
     *  +========================+
     *  | Image data preparation |
     *  +========================+
     */

    //  Load a raw chip image from path via OpenCV
    auto raw_image = cv::imread( image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );

    /*
     *  By default, OpenCV read the image with 8 bits RGB color from cv::imread
     *  We don't want OpenCV to reduce our raw chip image to the depth with 8 bits only
     *  So we set the parameter to:
     * 
     *      "cv::IMREAD_ANYCOLOR" for RGB color
     *                             or
     *      "cv::IMREAD_ANYDEPTH" for depth with our raw image (16 bits)
    */

    /*
     *  +========================+
     *  | ArUco code preparation |
     *  +========================+
     */

    //  Declare the ArUco ID and their relative order with raw chip image
    std::vector<std::int32_t> aruco_ids {
        47, 48, 49, 05, 50, 51, 52,
        40, 41, 42, 43, 44, 45, 46,
        34, 35, 36, 37, 38, 39, 04,
        28, 29, 03, 30, 31, 32, 33,
        21, 22, 23, 24, 25, 26, 27,
        15, 16, 17, 18, 19, 02, 20,
        00, 01, 10, 11, 12, 13, 14
    };
    nlohmann::json aruco_ids_map;
    for(std::size_t y = 0; y < 7; y ++) {
        for(std::size_t x = 0; x < 7; x ++) {
            auto key = aruco_ids.at((y * 7) + x);
            aruco_ids_map[std::to_string(key)] = {x, y};
        }
    }

    /*
     *  The coordinate is mapping start at top-left between the raw chip image and the vector of ArUco ID
     *  And each ArUco ID representes an ArUco marker on the raw chip image
     * 
     *      Raw Chip Image      Vector of ArUco ID
     *      +------------>      +------------>
     *      | []   []   []      | 47   48   49
     *      |                   |
     *      V []   []   []      V 40   41   42
     */

    //  Load ArUco database
    nlohmann::json aruco_database;
    std::ifstream aruco_database_file_in( aruco_database_path );
    aruco_database_file_in >> aruco_database;

    //  Set json dictionary to node "DICT_6X6_250"
    auto aruco_dict_6x6_250 = chipimgproc::aruco::Dictionary::from_json(
        aruco_database["DICT_6X6_250"]
    );

    /*
     *  DICT_6X6_250 means the dictionary of 250 ArUco code with size 6x6 in the json database
     */

    /*
     *  +============================+
     *  | Marker pattern preparation |
     *  +============================+
     */

    // create marker border template
    auto [templ, mask] = chipimgproc::aruco::create_location_marker(
        50, 40, 3, 5, 2.68
    );

    /// [data_preparation]
    /// [aruco_detection]

    /*
     *  +=================+
     *  | ArUco detection |
     *  +=================+
     */

    // Declare the ArUco marker detector 
    auto detector = chipimgproc::marker::detection::make_aruco_random(
        std::move(aruco_dict_6x6_250),     //  ArUco database
        templ, 
        mask,
        30 * 2.68, 
        2, 
        9, 
        50 * 2.68, 
        0.75
    );

    //  Detecting ArUco marker
    auto detected_aruco_id_position = detector(raw_image, aruco_ids);
/// [aruco_detection]

    /* 
     *  ArUco database:
     *      Database of 250 ArUco code with size 6x6
     * 
     *  Pyramid level:
     *      The "Level" parameter (unit: counting numbers) means the iteration times for doing down-smapling for speeding up the ArUco marker localization
     * 
     *  Border width:
     *      The distance (one unit: the side length of a cell) between coding region of ArUco and marker frame template
     * 
     *  Fringe width:
     *      The width (one unit: the side length of a cell) of marker frame template
     * 
     *  Bits width:
     *      The width (one unit: one pixel) of a cell
     * 
     *  Margin size:
     *      The width (one unit: one pixel) of the marker frame mask
     * 
     *  Marker frame template:
     *      Marker frame of inside pattern
     * 
     *  Marker frame mask:
     *      Marker frame of outside border
     * 
     *  Number of marker counts:
     *      The maximum number of counts (unit: counting numbers) of ArUco markers in an FOV
     *      For example, the number of marker counts is nine for the Yz01 and Banff chip
     * 
     *  Number of radius:
     *      The minimum distance (unit: pixel) between each ArUco markers
     *      That means there are no other ArUco markers of interest presenting in the circular region
     *      The radius of the circular region is user-specified
     * 
     *  Cell size:
     *      The width (unit: pixel) of the binary determination region
     * 
     *  ArUco IDs:
     *      The vector of ArUco IDs
     * 
     *  Logger:
     *      Log output
     * 
     */ 

    /// [aruco_detection]
    /// [output]

    /*
     *  +===================+
     *  | Output ArUco code |
     *  +===================+
     */
    
    // Outputing
    for( auto& [ id, score, position ] : detected_aruco_id_position )
        std::cout << id << '\t' << position << std::endl;

    /// [output]
    
    return 0;
}