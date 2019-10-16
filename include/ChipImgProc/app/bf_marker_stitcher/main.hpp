#pragma once
#include <iostream>
#include <stdexcept>
#include <nlohmann/json.hpp>
#include <Nucleona/language.hpp>
#include <Nucleona/algo/split.hpp>
#include <Nucleona/app/cli/option_parser.hpp>
#include <ChipImgProc/marker/detection/aruco_reg_mat.hpp>
#include <ChipImgProc/marker/detection/reg_mat_infer.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/stitch/utils.h>

namespace chipimgproc {
namespace app {
namespace bf_marker_stitcher{

struct Parameters
{
    std::string
          images_list
        , output_path
        , aruco_db_path
        , marker_temp_path
        , marker_mask_path
        , marker_patt_path
        ;

    cv::Mat
        marker_temp,
        marker_mask,
        marker,
        mask
        ;

    std::size_t x, y;
    nlohmann::json aruco_db;
    
    std::vector< cv::Mat > images;
    std::vector< std::int32_t > aruco_ids;
    std::vector< cv::Point > aruco_points;
};

class GetParameters :
    public Parameters,
    public nucleona::app::cli::OptionParser
{
public:
    GetParameters( int argc, char const * argv[] )
    {
        namespace po = boost::program_options;
        po::options_description desc( "Allowed options") ;
        desc.add_options()
            ( "help,h",    "show help message" )
            ( "images,i", po::value< std::string >()->required(), "The list file of image paths with Z path ordering" )
            ( "arucoj,j", po::value< std::string >()->required(), "The json file of aruco database" )
            ( "mktemp,t", po::value< std::string >()->required(), "Image path of marker frame template" )
            ( "mkmask,m", po::value< std::string >()->required(), "Image path of marker frame mask" )
            ( "mkpatt,p", po::value< std::string >()->required(), "The pattern file of CY3 marker" )
            ( "xnumbs,x", po::value< std::size_t >()->required(), "Number of images in X direction" )
            ( "ynumbs,y", po::value< std::size_t >()->required(), "Number of images in Y direction" )
            ( "output,o", po::value< std::string >()->required(), "The path to output the fully stitched image" )
        ;

        po::store( po::parse_command_line( argc, argv, desc ), vm );

        if( argc == 1 or vm.count( "help" ))
        {
            std::cout << desc << std::endl;
            std::exit(1);
        }

        po::notify( vm );
        
        get_parameter( "xnumbs", x );
        get_parameter( "ynumbs", y );
        get_parameter( "images", images_list );
        get_parameter( "output", output_path );
        get_parameter( "arucoj", aruco_db_path );
        get_parameter( "mktemp", marker_temp_path );
        get_parameter( "mkmask", marker_mask_path );
        get_parameter( "mkpatt", marker_patt_path );

        std::string image_path;
        std::ifstream list_file( images_list );
    
        while( std::getline( list_file, image_path ))
        {
            images.emplace_back(
                cv::imread( image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH )
            );
        }

        list_file.close();
        std::ifstream aruco_dbfile( aruco_db_path );
        aruco_dbfile >> aruco_db;

        aruco_ids = std::vector<std::int32_t>{
            47, 48, 49, 05, 50, 51, 52,
            40, 41, 42, 43, 44, 45, 46,
            34, 35, 36, 37, 38, 39, 04,
            28, 29, 03, 30, 31, 32, 33,
            21, 22, 23, 24, 25, 26, 27,
            15, 16, 17, 18, 19, 02, 20,
            00, 01, 10, 11, 12, 13, 14
        };

        aruco_points = std::vector< cv::Point >( 53 );

        for( int y = 0; y < 7; y++ )
            for( int x = 0; x < 7; x++ )
                aruco_points[ aruco_ids[ y * 7 + x ]] = cv::Point( x, y );
        
        marker_temp = cv::imread( marker_temp_path, cv::IMREAD_GRAYSCALE );
        marker_mask = cv::imread( marker_mask_path, cv::IMREAD_GRAYSCALE );

        std::ifstream marker_in( marker_patt_path );
        auto [ tp_marker, tp_mask ] = chipimgproc::marker::Loader::from_txt( marker_in );

        marker = tp_marker;
        mask   = tp_mask;
    }
};

template< class GET_PARAMETERS >
void Run( GET_PARAMETERS&& args )
{
    std::vector< cv::Point > points;
    chipimgproc::marker::detection::ArucoRegMat marker_detector;

    chipimgproc::rotation::MarkerVec<float> theta_detector;
    chipimgproc::rotation::Calibrate image_rotator;

    marker_detector.set_dict( args.aruco_db_path, "DICT_6X6_250" );
    marker_detector.set_detector_ext(
        3,                     //  Pyramid level
        1,                     //  Border width
        1,                     //  Fringe width
        13.4,                  //  Bits width
        8.04,                  //  Margin size
        args.marker_temp_path, //  marker frame template
        args.marker_mask_path, //  Marker frame mask
        9,                     //  Number of marker counts
        268,                   //  Number of radius
        5,                     //  Cell size
        args.aruco_ids,        //  VArUco IDs
        args.aruco_points
    );

    auto layout = chipimgproc::marker::make_single_pattern_reg_mat_layout(
        args.marker, //  Marker
        args.mask,   //  Mask
        4,           //  Height (µm) of cell
        4,           //  Width (µm) of cell
        1,           //  Spacing (µm) of cell
        3,           //  Row number of markers
        3,           //  Column number of markers
        81,          //  Spacing x of marker
        81,          //  Spacing y of marker
        2.68         //  µm to pixel
    );
    
    for( auto&& image : args.images )
    {
        auto aruco_mk = marker_detector( image, layout, chipimgproc::MatUnit::PX );
        auto theta = theta_detector( aruco_mk );

        image_rotator( image, theta );

        cv::Rect roi{ 100, 100, image.cols -200, image.rows -200 };
        image = image( roi );

        cv::Mat_< std::uint16_t > img = image.clone();

        aruco_mk = marker_detector( image, layout, chipimgproc::MatUnit::PX );
        aruco_mk = chipimgproc::marker::detection::reg_mat_infer( aruco_mk, 3, 3, img );
        //                            Marker numbers in both row and column ^  ^

        points.emplace_back( cv::Point( aruco_mk[0].x, aruco_mk[0].y ));
    }

    std::cerr << "\n";
    for( auto&& point : points ) std::cerr << point << "\n";
    std::cerr << "\n";

    points[0] = cv::Point( points[0].x, points[0].y );
    points[1] = cv::Point( points[1].x-30, points[1].y-17 );
    points[2] = cv::Point( points[2].x-15, points[2].y-35 ); // 25

    std::cerr << "\n";
    for( auto&& point : points ) std::cerr << point << "\n";
    std::cerr << "\n";

    cv::Mat full_image = chipimgproc::stitch::add( args.images, points );
    cv::imwrite( args.output_path, full_image );
}

}}}
