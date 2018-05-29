#pragma once
#include <Nucleona/language.hpp>
#include <iostream>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/stitch/position_based.hpp>
#include <Nucleona/app/cli/option_parser.hpp>
#include <stdexcept>
#include <Nucleona/algo/split.hpp>
#include <Nucleona/format/csv_parser.hpp>
namespace chipimgproc {
namespace app {
namespace position_stitcher{
struct Parameters
{
    std::string img_pos_list_file_path;
    int x;
    int y;
    std::string output_image_path;
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
            ("im_list,l"    , po::value<std::string>()->required()  , "image list going to be stitiched, the order must follow Z shape walk.")
            ("x_axis,x"     , po::value<int>()->required()          , "number of images along x axis")
            ("y_axis,y"     , po::value<int>()->required()          , "number of images along y axis")
            ("output,o"     , po::value<std::string>()->required()  , "output image path")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 or vm.count("help"))
        {
            std::cout << desc << std::endl;
            std::exit(1);
        }
        po::notify(vm);
        get_parameter ("im_list"     , img_pos_list_file_path);
        get_parameter ("x_axis"      , x);
        get_parameter ("y_axis"      , y);
        get_parameter ("output"      , output_image_path);
    }
};

template<class OPTION_PARSER>
class Main
{
    // using OPTION_PARSER = OptionParser;
    struct ImgListEntry {
        std::string path;
        int x;
        int y;
    };
    using ImgListParser = nucleona::format::CsvParser<
        nucleona::format::CsvEntryTrait<
            ImgListEntry,
            nucleona::format::PMT<0, std::string>,
            nucleona::format::PMT<1, int>,
            nucleona::format::PMT<2, int>
        >
    >;
    struct StitcherParam {
        std::vector<cv::Mat> imgs;
        std::vector<cv::Point_<int>> pos;
    };
    ImgListParser img_list_parser_;
    OPTION_PARSER args_;
  public:
    Main( OPTION_PARSER&& args )
    : args_( std::forward<OPTION_PARSER>( args ) )
    , img_list_parser_("\t")
    {}
    StitcherParam get_images() {
        std::ifstream fin(args_.img_pos_list_file_path);
        StitcherParam params;
        std::string line;
        ImgListEntry entry;
        bool first = true;
        int r = 0, c = 0;
        while(std::getline(fin, line)) {
            entry = img_list_parser_(line);
            cv::Mat tmp = cv::imread(entry.path, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            cv::Mat mat;
            cv::extractChannel(tmp, mat, tmp.channels() - 1);
            if( first ) {
                r = mat.rows;
                c = mat.cols;
                first = false;
            } else {
            }
            params.imgs.push_back(mat);
            params.pos.emplace_back(entry.x, entry.y);
        }
        return params;
    }
    void operator()() {
        auto imgs = get_images();
        stitch::PositionBased pb(args_.y, args_.x);
        cv::Mat out = pb(imgs.imgs, imgs.pos);
        cv::imwrite(args_.output_image_path, out);
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
