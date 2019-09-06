#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/loader.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <boost/program_options.hpp>

/*
 *  This example code will help you to build the image rotation app. 
 *
 *  Input:
 *          TIFF file of the raw image from SUMMIT
 * 
 *          TSV file of marker pattern
 *          
 *  output:
 *          The degree of the image rotated theta
 * 
 *  Example:
 *          ./Example-aruco_detection
 *              -i banff_test/0-0-2.tiff
 *              -m banff_rc/pat_CY3.tsv
 */

int main( int argc, char** argv )
{
    /*
     *  +=========================+
     *  | Declare program options |
     *  +=========================+
     */

    std::string raw_image_path;         //  The path of raw chip image
    std::string marker_pattern_path;    //  The path of marker pattern

    boost::program_options::variables_map op;
    boost::program_options::options_description options( "Options" );

    options.add_options()( "help,h" , "Print this help messages" )
        ( "image,i" , boost::program_options::value< std::string >( &raw_image_path      )->required(),"Image path of raw chip image" )
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
     *  +========================+
     *  | Image data preparation |
     *  +========================+
     */

    //  Load raw chip image from path via OpenCV
    cv::Mat_<std::uint16_t> raw_image = cv::imread( raw_image_path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH );

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


    // Load marker
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

    // Declare the marker detector and image rotator
    chipimgproc::marker::detection::RegMat marker_detector;
    chipimgproc::rotation::MarkerVec<float> image_rotator;
    
    //  Detecting marker
    auto mk_regs = marker_detector( raw_image, marker_layout, chipimgproc::MatUnit::PX, 0, std::cout );
    
    //  Rotating image
    auto theta = image_rotator( mk_regs, std::cout );

    /*
     *  +========================+
     *  | Output rotation result |
     *  +========================+
     */

    // Outputing
    std::cout << theta << std::endl;

    return 0;
}