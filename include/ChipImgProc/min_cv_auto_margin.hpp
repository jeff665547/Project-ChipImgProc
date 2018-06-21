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
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/utils/cell_stat.hpp>
#include <algorithm>

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
    CellStat count_cv ( const cv::Mat_<int32_t>& mat )
    {
        CellStat res;
        cv::Scalar_<double> mean, stddev;
        cv::meanStdDev( mat, mean, stddev );
        
        res.mean = mean(0);
        res.stddev = stddev(0);
        res.cv = res.stddev / res.mean;
        return res;
    }
    auto find_min_cv(
          const cv::Mat& src
        , const cv::Rect& t
        , std::int32_t windows_width
        , std::int32_t windows_height 
    ) {

        CellStat min;
        cv::Rect min_cv_win(0, 0, windows_width, windows_height);
        min.cv = std::numeric_limits<float>::max();
        for ( std::int32_t i = t.y; i < t.y + t.height - windows_height + 1; i ++ )
        {
            for ( std::int32_t j = t.x; j < t.x + t.width - windows_width + 1; j ++ )
            {
                cv::Rect_<int32_t> rect(
                    j, i, 
                    windows_width, windows_height
                );

                auto window = src( rect );
                // integer_check( window );
                auto win_res_ele = count_cv( window );
                if ( win_res_ele.cv < min.cv )
                {
                    min = win_res_ele;
                    min_cv_win.x = j;
                    min_cv_win.y = i;
                }
            }
        }
        min.num = windows_height * windows_width;
        if( min_cv_win.x < 0 )       { throw std::runtime_error("min cv tile constrain check fail"); };
        if( min_cv_win.y < 0 )       { throw std::runtime_error("min cv tile constrain check fail"); };
        if( min_cv_win.width <= 0 )  { throw std::runtime_error("min cv tile constrain check fail"); };
        if( min_cv_win.height <= 0 ) { throw std::runtime_error("min cv tile constrain check fail"); };
        // min.detail_raw_value = src(t);
        struct {
            CellStat stat;
            cv::Rect win;
        } res {
            min, min_cv_win
        };
        return res;
    }
    /**
     * @brief   @copybrief ChipImgProc/min_cv_auto_margin.hpp
     * @param   tiled_src       Image data and grid cell generate by gridding step.
     * @param   windows_width   The result section width in grid cell after auto margin.
     * @param   windows_height  The result section height in grid cell after auto margin.
     */
    template<class TID, class GLID>
    auto operator()( 
          TiledMat<TID, GLID>& tiled_src
        , std::int32_t windows_width
        , std::int32_t windows_height 
    )
    {
        std::vector<cv::Mat> stat(3, cv::Mat_<float>(
            tiled_src.rows, tiled_src.cols
        ) );
        stat.push_back(cv::Mat_<std::uint32_t>(
            tiled_src.rows, tiled_src.cols
        ));
        auto& tiles = tiled_src.get_tiles();
        for( int y = 0; y < tiled_src.rows; y ++ ) {
            for ( int x = 0; x < tiled_src.cols; x ++ ) {
                auto& t = tiled_src.tile_at(x, y);
                auto min_cv_data = find_min_cv(tiled_src.get_cali_img(), t, windows_width, windows_width);
                stat[StatIdx::means]  (x, y) = min_cv_data.stat.mean;
                stat[StatIdx::stddev] (x, y) = min_cv_data.stat.stddev;
                stat[StatIdx::cv]     (x, y) = min_cv_data.stat.cv;
                stat[StatIdx::num]    (x, y) = min_cv_data.stat.num;
                t = min_cv_data.win;
            }
        }
        return stat;
    };
};

}
