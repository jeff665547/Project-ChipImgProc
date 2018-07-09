#pragma once
#include <ChipImgProc/const.h>
namespace chipimgproc{ 

constexpr struct CoordinateFix {
    template<class T>
    auto operator()(
        cv::Mat_<T>& m          ,
        const std::string& op   ,
        const std::string& x_direct
    ) const {
        if      ( op == OriginPos::LT && x_direct == XDirect::H )
        {}
        else if ( op == OriginPos::LT && x_direct == XDirect::V )
        {
            cv::transpose( m, m );
        }
        else if ( op == OriginPos::LB && x_direct == XDirect::H )
        { // flip upside down
            cv::flip ( m, m, 0 );
        }
        else if ( op == OriginPos::LB && x_direct == XDirect::V )
        { // rotate 90 degree
            transpose(m, m);  
            flip( m, m, 1 );
        }
        else
        {
            throw std::logic_error( "unknown coordinate system" );
        }
    }
} coordinate_fix;

}