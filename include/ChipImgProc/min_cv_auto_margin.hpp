/**
 *  @file    ChipImgProc/min_cv_auto_margin.hpp
 *  @author  Chia-Hua Chang
 *  @brief   Adjustment the pixel padding and margin size of grid by searching the minimum coefficient of variation section.
 */
#pragma once
#include <cstdint>
#include <vector>
#include <ChipImgProc/utils.h>
#include <cassert>
#include <Nucleona/util.hpp>

namespace chipimgproc {
/**
 *  @brief      @copybrief ChipImgProc/min_cv_auto_margin.hpp
 *  @details    Search the minimum coefficient of variation section in the grid cell, 
 *              use the selected section to represent the cell value. 
 *              The section size is defined by input parameter.
 *              See @ref improc_min_cv_auto_margin for more detail.
 */
struct MinCVAutoMargin
{
    struct ResEle
    {
        float mean;
        float stddev;
        float cv_value;
        cv::Mat_<int32_t> detail_raw_value;
    };
    auto count_cv ( const cv::Mat_<int32_t>& mat )
    {
        ResEle res;
        cv::Scalar_<double> mean, stddev;
        cv::meanStdDev( mat, mean, stddev );
        
        res.mean = mean(0);
        res.stddev = stddev(0);
        res.cv_value = res.stddev / res.mean;
        res.detail_raw_value = mat;
        // for( auto v : mat )
        // {
        //     assert( v >= 0 );
        // }
        return res;
    }
    /**
     * @brief   @copybrief ChipImgProc/min_cv_auto_margin.hpp
     * @param   src             Image data.
     * @param   tiles           The grid cell generate by gridding step.
     * @param   windows_width   The result section width in grid cell after auto margin.
     * @param   windows_height  The result section height in grid cell after auto margin.
     */
    auto operator()( 
          const cv::Mat& src
        , std::vector<cv::Rect>& tiles
        , std::int32_t windows_width
        , std::int32_t windows_height 
    )
    {
        // std::cout << cpt::improc::depth( src ) << std::endl;
        std::vector<ResEle> res;
        res.reserve(tiles.size());
        for ( auto& t: tiles )
        {
            if( t.width < windows_width   ) throw std::runtime_error("window width value too big must smaller then tile.");
            if( t.height < windows_height ) throw std::runtime_error("window height value too big must smaller then tile.");

            cv::Rect_<int32_t> rect;
            rect.width = windows_width;
            rect.height = windows_height;

            cv::Rect_<int32_t> min_cv_windows;
            min_cv_windows.width = windows_width;
            min_cv_windows.height = windows_height;

            ResEle min;
            min.cv_value = std::numeric_limits<float>::max();
            for ( int32_t i = t.y; i < t.y + t.height - windows_height; i ++ )
            {
                for ( int32_t j = t.x; j < t.x + t.width - windows_width; j ++ )
                {
                    rect.x = j;
                    rect.y = i;

                    auto window = src( rect );
                    // integer_check( window );
                    auto win_res_ele = count_cv( window );
                    if ( win_res_ele.cv_value < min.cv_value )
                    {
                        min = win_res_ele;
                        min_cv_windows.x = j;
                        min_cv_windows.y = i;
                    }
                }
            }
            t = min_cv_windows;
            assert( t.x >= 0 );
            assert( t.y >= 0 );
            assert( t.width > 0 );
            assert( t.height > 0 );
            // min.detail_raw_value = src(t);
            res.push_back( min );
        }
        return res;
    };
};

}
