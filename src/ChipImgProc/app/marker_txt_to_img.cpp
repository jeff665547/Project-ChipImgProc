#include <ChipImgProc/app/marker_txt_to_img/main.hpp>

int main( int argc, const char* argv[] )
{
    chipimgproc::app::marker_txt_to_img::OptionParser option_parser(argc, argv);    
    auto&& converter( 
        chipimgproc::app::marker_txt_to_img::make( option_parser )
    );
    converter();
}