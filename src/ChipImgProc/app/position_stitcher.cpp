#include <iostream> 
#include <string>
#include <ChipImgProc/app/position_stitcher/main.hpp>
int main( int argc, const char* argv[] )
{
    chipimgproc::app::position_stitcher::OptionParser option_parser(argc, argv);    
    auto&& stitcher( 
        chipimgproc::app::position_stitcher::make( option_parser )
    );
    stitcher();
}
