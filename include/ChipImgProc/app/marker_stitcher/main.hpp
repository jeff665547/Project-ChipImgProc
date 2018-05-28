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
namespace chipimgproc {
namespace app {
namespace marker_stitcher{
struct Parameters
{
    std::string img_list_file_path;
    int x;
    int y;
    std::string output_image_path;
    std::string marker_path;
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
            ("marker,m"     , po::value<std::string>()->required()  , "marker image path")
        ;
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(argc == 1 or vm.count("help"))
        {
            std::cout << desc << std::endl;
            std::exit(1);
        }
        po::notify(vm);
        get_parameter ("im_list"     , img_list_file_path);
        get_parameter ("x_axis"      , x);
        get_parameter ("y_axis"      , y);
        get_parameter ("output"      , output_image_path);
        get_parameter ("marker"      , marker_path);
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
    std::vector<cv::Mat> get_images() {
        std::ifstream fin(args_.img_list_file_path);
        std::vector<cv::Mat> image_list;
        std::string line;
        bool first = true;
        int r = 0, c = 0;
        while(std::getline(fin, line)) {
            cv::Mat tmp = cv::imread(line, cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            cv::Mat mat;
            cv::extractChannel(tmp, mat, tmp.channels() - 1);
            if( first ) {
                r = mat.rows;
                c = mat.cols;
                first = false;
            } else {
            }
            image_list.push_back(mat);
        }
        return image_list;
    }
    static cv::Mat_<uint8_t> load_marker(const boost::filesystem::path& path_to_marker)
    {
        if (!boost::filesystem::exists(path_to_marker))
        {
            throw std::runtime_error(path_to_marker.string() + " not found!");
        }
        std::cout << "found " << path_to_marker << std::endl;

        cv::Mat_<uint8_t> marker ;// = decltype(marker)::zeros(10, 10);
        std::ifstream is(path_to_marker.string());
        std::string line;
        int cols = 0;
        int rows = 0;
        while( std::getline( is, line ) )
        {
            auto marks = nucleona::algo::split( line, " " );
            if( cols == 0 ) cols = marks.size();
            else {
                if( cols != marks.size() ) {
                    throw std::runtime_error("assert fail");
                }
            }
            for ( auto&& c : marks )
            {
                if( c == "X" ) marker.push_back( (uint8_t)255 );
                else if ( c == "." ) marker.push_back( (uint8_t)0 );
                else throw std::runtime_error( "unknown marker character: " + c );
            }
            rows ++;
        }
        marker = marker.reshape(1, rows );

        is.clear();
        is.close();

        return marker;
    }
    cv::Mat get_marker() {
        boost::filesystem::path p(args_.marker_path);
        auto raw_marker = load_marker(p);
        cv::Mat_<std::uint16_t> kernel(cv::Size(29, 29), 0);
        kernel(cv::Rect(3,3, 23, 23)) = 1;
        cv::Mat res;
        res.create(
            raw_marker.rows * kernel.rows,
            raw_marker.cols * kernel.cols,
            kernel.type()
        );
        for ( auto r = 0; r != raw_marker.rows; ++r ) {
            for (auto c = 0; c != raw_marker.cols; ++c) {
                cv::Range row(r * kernel.rows, (r + 1) * kernel.rows);
                cv::Range col(c * kernel.cols, (c + 1) * kernel.cols);
                res(row, col) = kernel * raw_marker(r,c);
            }
        }
        return res;
    }
    cv::Mat_<float> match_template(cv::Mat img, cv::Mat marker) {
        cv::Mat_<float> sm(
            img.rows - marker.rows + 1,
            img.cols - marker.cols + 1
        );
        if( img.type() == CV_16U ) {
            cv::Mat tmp;
            img.convertTo(tmp, CV_8U, 1.0 / 257);
            img = tmp;
        }
        if ( marker.type() == CV_16U ) {
            cv::Mat tmp;
            marker.convertTo(tmp, CV_8U, 1.0/257);
            marker = tmp;
        }

        cv::matchTemplate(img, marker, sm, cv::TM_CCORR_NORMED);
        return sm;
    }
    std::vector<cv::Point> feature_extract( const cv::Mat& img, const cv::Mat& marker) {
        std::vector<cv::Point> res;
        int seg_width = img.cols / 3;
        int seg_height = img.rows / 3;
        for ( int i = 0; i < ( img.rows - 2 ); i += seg_height ) {
            for( int j = 0; j < ( img.cols - 2 ); j += seg_width ) {
                cv::Mat sub_img = img(cv::Rect(j, i, seg_width, seg_height));
                // cv::Mat_<float> sm(
                //     sub_img.rows - marker.rows + 1,
                //     sub_img.cols - marker.cols + 1
                // );
                // cv::matchTemplate(sub_img, marker, sm, cv::TM_CCORR_NORMED);
                auto sm = match_template( sub_img.clone(), marker.clone() );
                // {
                //     cv::Mat tmp;
                //     sm.convertTo(tmp, CV_8U, 255);
                //     cv::imwrite("debug.tiff", tmp);
                // }
                auto p = arg_max(sm);
                res.push_back({p.x + j, p.y + i});
            }
        }
        return res;
    }
    template<class T>
    cv::Point arg_max( cv::Mat_<T>& mat ) {
        cv::Point max_p(0, 0);
        T max_v = mat.template at<T>(max_p);
        for(int i = 0; i < mat.rows; i ++ ) {
            for( int j = 0; j < mat.cols; j ++ ) {
                cv::Point p(j, i);
                auto v = mat.template at<T>(p);
                if( max_v < v ) {
                    max_v = v;
                    max_p = p;
                }
            }
        }
        return max_p;
    }
    void operator()() {
        /** 
         * 1. feature extract
         * 2. row stitch
         * 3. col stitch
         */
        auto imgs = get_images();
        auto marker = get_marker();
        std::vector<
            std::vector<cv::Point>
        > feats;
        {
            int i = 0;
            for ( auto&& m : imgs ) {
                auto m_feats = feature_extract(m, marker);
                {
                    auto tmp = m.clone();
                    for ( auto&& f : m_feats ) {
                        cv::rectangle(tmp, cv::Rect(
                            f.x, f.y, 50, 50
                        ), cv::Scalar(0xFFFF,0xFFFF,0xFFFF),3);
                    }
                }
                feats.push_back(m_feats);
                i++;
            }
        }
        stitch::MarkerBased ms(args_.y, args_.x);
        cv::Mat out = ms(imgs, feats, 3, 3);
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
