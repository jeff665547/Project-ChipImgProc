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
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <algorithm>

namespace chipimgproc { namespace margin{
/**
 *  @brief      @copybrief ChipImgProc/min_cv_auto_margin.hpp
 *  @details    Search the minimum coefficient of variation section in the grid cell, 
 *              use the selected section to represent the cell value. 
 *              The section size is defined by input parameter.
 *              See @ref improc_min_cv_auto_margin for more detail.
 */
template<class FLOAT>
struct AutoMinCV
{
    stat::Cell<FLOAT> count_cv ( const cv::Mat& mat ) const 
    {
        return stat::Cell<FLOAT>::make(mat);
    }
    auto find_min_cv(
          const cv::Mat& src
        , const cv::Rect& t
        , std::int32_t windows_width
        , std::int32_t windows_height 
    ) const {
        // TODO: optimize
        stat::Cell<FLOAT> min;
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
            stat::Cell<FLOAT> stat;
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
     * @param   v_result        The process result debug image view.
     */
    template<class GLID>
    auto operator()( 
          TiledMat<GLID>&           tiled_src
        , float                     seg_rate
        , bool                      tile_replace        = true
        , const std::function<
            void(const cv::Mat&)
          >&                        v_result            = nullptr
    ) const 
    {
        stat::Mats<FLOAT> res(rows(tiled_src), cols(tiled_src));
        auto& tiles = tiled_src.get_tiles();
        float basic_margin_rate = 0.17;
        for( int y = 0; y < rows(tiled_src); y ++ ) {
            for ( int x = 0; x < cols(tiled_src); x ++ ) {
                auto t = tiled_src.tile_at(y, x);
                auto x_basic_margin = basic_margin_rate * t.width;
                auto y_basic_margin = basic_margin_rate * t.height;
                t.x += x_basic_margin;
                t.y += y_basic_margin;
                t.width -= (2 * x_basic_margin);
                t.height -= (2 * y_basic_margin);
                int windows_width  = std::round(t.width  * seg_rate);
                int windows_height = std::round(t.height * seg_rate);
                auto min_cv_data = find_min_cv(
                    tiled_src.get_cali_img(), t, 
                    windows_width, windows_height
                );
                res.mean   (y, x) = min_cv_data.stat.mean;
                res.stddev (y, x) = min_cv_data.stat.stddev;
                res.cv     (y, x) = min_cv_data.stat.cv;
                res.num    (y, x) = min_cv_data.stat.num;
                if( tile_replace )
                    tiled_src.tile_at(y, x) = min_cv_data.win;
            }
        }
        auto& mat = tiled_src.get_cali_img();
        if(mat.depth() == CV_32F || mat.depth() == CV_64F){
            auto tmp = mat.clone();
            tmp.convertTo(
                mat, CV_16UC1, 1.0
            );
        }
        if(v_result) {
            tiled_src.view(v_result);
        }
        return res;
    };
};

}
}