#pragma once
#include <Nucleona/language.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/stitch/marker_based.hpp>
#include <Nucleona/app/cli/option_parser.hpp>
#include <stdexcept>
#include <Nucleona/algo/split.hpp>
#include <ChipImgProc/marker/loader.hpp>
namespace chipimgproc {
namespace app {
namespace marker_txt_to_img{
struct Parameters
{
    std::string txt_input;
    std::string img_output;
    int cell_r_px;
    int cell_c_px;
    int border_px;
};

class OptionParser : public Parameters, public nucleona::app::cli::OptionParser
{
public:
    OptionParser(int argc, char const * argv[])
    {
        namespace po = boost::program_options;
        po::options_description desc("Allowed options");
        desc.add_options()
            ("help,h"       , "show help message")
            ("txt_input,i"  , po::value<std::string>()->required()  , "marker txt pattern file")
            ("img_output,o" , po::value<std::string>()->required()  , "marker image output")
            ("cell_r_px,r"  , po::value<int>()->required()          , "cell row pixel num")
            ("cell_c_px,c"  , po::value<int>()->required()          , "cell col pixel num")
            ("border_px,b"  , po::value<int>()->required()          , "cell border pixel num")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 or vm.count("help"))
        {
            std::cout << desc << std::endl;
            std::exit(1);
        }
        po::notify(vm);
        get_parameter ("txt_input" , txt_input  );  
        get_parameter ("img_output", img_output );   
        get_parameter ("cell_r_px" , cell_r_px  );  
        get_parameter ("cell_c_px" , cell_c_px  );  
        get_parameter ("border_px" , border_px  ); 
    }
};

template<class OPTION_PARSER>
class Main
{
    // using OPTION_PARSER = OptionParser;
    OPTION_PARSER args_;
  public:
    Main( OPTION_PARSER&& args )
    : args_( std::forward<OPTION_PARSER>( args ) )
    {}
    void operator()() {
        std::ifstream fin(args_.txt_input);
        auto mat = marker::Loader::from_txt(fin, std::cout);
        auto r_cell_bd = args_.cell_r_px + args_.border_px;
        auto c_cell_bd = args_.cell_c_px + args_.border_px;
        auto img_row = 
            args_.border_px + 
            r_cell_bd * mat.rows;
        auto img_col = 
            args_.border_px + 
            c_cell_bd * mat.cols;

        cv::Mat_<std::uint8_t> img = cv::Mat_<std::uint8_t>::zeros(
            img_row, img_col
        );
        int i = 0;
        for( int r = args_.border_px; r < img.rows; r += r_cell_bd ) {
            int j = 0;
            for( int c = args_.border_px; c < img.cols; c += c_cell_bd ) {
                if( mat(i,j) == 255 ) {
                    cv::Rect cell(c, r, args_.cell_c_px, args_.cell_r_px);
                    cv::rectangle(img, cell, 255, CV_FILLED);
                }
                j ++;
            }
            i ++;
        }
        cv::imwrite(args_.img_output, img);
    }
};

template<class OPTION_PARSER>
auto make ( OPTION_PARSER&& option_parser )
{
    return Main<OPTION_PARSER> ( 
        std::forward < OPTION_PARSER > ( option_parser )
    );
}

}}}
