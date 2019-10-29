#include <iostream> 
#include <string>
#include <ChipImgProc/app/bf_marker_stitcher/main.hpp>

int main( int argc, const char* argv[] )
{
    chipimgproc::app::bf_marker_stitcher::GetParameters parameters( argc, argv );    
    chipimgproc::app::bf_marker_stitcher::Run( parameters );
    return 0;
}