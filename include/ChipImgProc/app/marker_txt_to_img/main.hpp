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
#include <ChipImgProc/marker/txt_to_img.hpp>
namespace chipimgproc {
namespace app {
namespace marker_txt_to_img{
struct Parameters
{
    std::string txt_input;
    std::string img_output;
    float cell_r_px;
    float cell_c_px;
    float border_px;
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
            ("cell_r_px,r"  , po::value<float>()->required()        , "cell row pixel num")
            ("cell_c_px,c"  , po::value<float>()->required()        , "cell col pixel num")
            ("border_px,b"  , po::value<float>()->required()        , "cell border pixel num")
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
        auto [mat, mask] = marker::Loader::from_txt(fin, std::cout);
        auto [img, mask_img] = txt_to_img(mat, mask, 
            args_.cell_r_px,
            args_.cell_c_px,
            args_.border_px
        );
        cv::imwrite(args_.img_output, img);
    }

    chipimgproc::marker::TxtToImg txt_to_img;
};

template<class OPTION_PARSER>
auto make ( OPTION_PARSER&& option_parser )
{
    return Main<OPTION_PARSER> ( 
        std::forward < OPTION_PARSER > ( option_parser )
    );
}

}}}
