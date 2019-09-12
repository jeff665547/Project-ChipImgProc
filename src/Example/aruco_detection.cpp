#include <nlohmann/json.hpp>
#include <ChipImgProc/aruco.hpp>
#include <boost/program_options.hpp>

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

    std::string marker_frame_template_path; //  The path of marker frame template image
    std::string marker_frame_mask_path;     //  The path of marker frame mask image

    boost::program_options::variables_map op;
    boost::program_options::options_description options( "Options" );

    options.add_options()( "help,h" , "Print this help messages" )
        ( "image,i"    , boost::program_options::value< std::string >( &image_path                 )->required(),"Image path of raw chip image" )
        ( "json,j"     , boost::program_options::value< std::string >( &aruco_database_path        )->required(),"Json path of ArUco database" )
        ( "template,t" , boost::program_options::value< std::string >( &marker_frame_template_path )->required(),"Image path of marker frame template" )
        ( "mask,m"     , boost::program_options::value< std::string >( &marker_frame_mask_path     )->required(),"Image path of marker frame mask" )
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
     *  +========================+
     *  | Image data preparation |
     *  +========================+
     */

    //  Load raw chip image from path via OpenCV
    auto raw_image = cv::imread( image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );

    /*
     *  By default, OpenCV read the image with 8 bits RGB color from cv::imread
     *  We don't want OpenCV reduce our raw chip image to the depth with 8 bits only
     *  So we give the parameter and set to:
     * 
     *      "cv::IMREAD_ANYCOLOR" for RGB color
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

    /*
     *  The coordinate is mapping start at top-left between the raw chip image and the vector of ArUco ID
     *  And echa ArUco ID represente to an ArUco marker on the raw chip image
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

    //  Load marker frame images of both template and mask
    auto marker_frame_template = cv::imread( marker_frame_template_path, cv::IMREAD_GRAYSCALE );
    auto marker_frame_mask     = cv::imread( marker_frame_mask_path,     cv::IMREAD_GRAYSCALE );
/// [data_preparation]
/// [aruco_detection]
    /*
     *  +=================+
     *  | ArUco detection |
     *  +=================+
     */

    // Declare the ArUco marker detector 
    chipimgproc::aruco::Detector detector;
    detector.reset(
        aruco_dict_6x6_250,     //  ArUco database
        3,                      //  Pyramid level
        1,                      //  Border bits
        1,                      //  Fringe bits
        13.4,                   //  Bits width
        8.04,                   //  Margin size
        marker_frame_template,  //  marker frame template
        marker_frame_mask,      //  Marker frame mask
        9,                      //  Number of marker counts
        268,                    //  Number of radius
        5,                      //  Cell size
        aruco_ids,              //  VArUco IDs
        std::cout               //  Logger
    );

    //  Detecting ArUco marker
    auto detected_aruco_id_position = detector.detect_markers( raw_image, std::cout );
/// [aruco_detection]

    /* 
     *  ArUco database:
     *      Database of 250 ArUco code with size 6x6
     * 
     *  Pyramid level:
     *      A level for down-sampling which to speed up the ArUco marker detection
     * 
     *  Border bits:
     *      A bit distance between coding region of ArUco and  marker frame template
     * 
     *  Fringe bits:
     *      A bit width of marker frame template
     * 
     *  Bits width:
     *      A bit width of each pixels
     * 
     *  Margin size:
     *      A bit with of marker frame mask
     * 
     *  Marker frame template:
     *      Marker frame inside pattern
     * 
     *  Marker frame mask:
     *      Marker frame outside border
     * 
     *  Number of marker counts:
     *      Maximum count of ArUco markers
     * 
     *  Number of radius:
     *      Minimum distance between each ArUco markers
     * 
     *  Cell size:
     *      A bit with of binary determination region
     * 
     *  ArUco IDs:
     *      Vector of ArUco IDs
     * 
     *  Logger:
     *      Log output
     * 
     */ 
/// [output]
    /*
     *  +===================+
     *  | Output ArUco code |
     *  +===================+
     */
    
    // Outputing
    for( auto& [ id, position ] : detected_aruco_id_position )
        std::cout << id << '\t' << position << std::endl;

/// [output]
    return 0;
}