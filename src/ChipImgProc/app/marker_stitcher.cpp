#include <iostream> 
#include <string>
#include <ChipImgProc/app/marker_stitcher/main.hpp>
int main( int argc, const char* argv[] )
{
    chipimgproc::app::marker_stitcher::OptionParser option_parser(argc, argv);    
    auto&& stitcher( 
        chipimgproc::app::marker_stitcher::make( option_parser )
    );
    stitcher();
}