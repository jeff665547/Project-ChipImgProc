/**
 *  @file    ChipImgProc/min_cv_auto_margin.hpp
 *  @author  Chia-Hua Chang
 *  @brief   Adjustment the pixel padding and margin size of grid by searching the minimum coefficient of variation section.
 */
#pragma once
#include <cstdint>
#include <vector>
#include <map>
#include <tuple>
#include <cmath>
#include <ChipImgProc/utils.h>
#include <cassert>
#include <Nucleona/util.hpp>
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/algo/partial_percentile.hpp>
#include <algorithm>

namespace chipimgproc { namespace margin{

template<class FLOAT>
struct SigEst
{
    // stat::Cell<FLOAT> count_cv ( const cv::Mat& mat ) const 
    // {
    //     auto tmp = stat::Cell<FLOAT>::make(mat);
    //     tmp.bg = 5;
    //     return tmp;
    // }
    // auto find_min_cv(
    //       const cv::Mat& src
    //     , const cv::Rect& t
    //     , std::int32_t windows_width
    //     , std::int32_t windows_height 
    // ) const {
    //     // TODO: optimize
    //     stat::Cell<FLOAT> min;
    //     cv::Rect min_cv_win(0, 0, windows_width, windows_height);
    //     min.cv = std::numeric_limits<float>::max();
    //     for ( std::int32_t i = t.y; i < t.y + t.height - windows_height + 1; i ++ )
    //     {
    //         for ( std::int32_t j = t.x; j < t.x + t.width - windows_width + 1; j ++ )
    //         {
    //             cv::Rect_<int32_t> rect(
    //                 j, i, 
    //                 windows_width, windows_height
    //             );
    // 
    //             auto window = src( rect );
    //             auto win_res_ele = count_cv( window );
    //             if ( win_res_ele.cv < min.cv )
    //             {
    //                 min = win_res_ele;
    //                 min_cv_win.x = j;
    //                 min_cv_win.y = i;
    //             }
    //         }
    //     }
    //     min.num = windows_height * windows_width;
    //     if( min_cv_win.x < 0 )       { throw std::runtime_error("min cv tile constrain check fail"); };
    //     if( min_cv_win.y < 0 )       { throw std::runtime_error("min cv tile constrain check fail"); };
    //     if( min_cv_win.width <= 0 )  { throw std::runtime_error("min cv tile constrain check fail"); };
    //     if( min_cv_win.height <= 0 ) { throw std::runtime_error("min cv tile constrain check fail"); };
    //     // min.detail_raw_value = src(t);
    //     struct {
    //         stat::Cell<FLOAT> stat;
    //         cv::Rect win;
    //     } res {
    //         min, min_cv_win
    //     };
    //     return res;
    // }
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
        double frank = 0.90, foffset = 0.40 * 0.5;
        double brank = 0.05, boffset = 0.25 * 0.5;
        
        std::map<
            std::tuple<int32_t,int32_t>
          , std::tuple<
                int32_t
              , int32_t
              , cv::Rect
              , std::vector<cv::Point>
              , std::vector<cv::Point>
            >
        > cache;

        chipimgproc::PartialPercentile<uint16_t> partial_percentile;
        auto&& matx = tiled_src.get_cali_img();
        for (auto r = 0; r != rows(tiled_src); ++r) {
            for (auto c = 0; c != cols(tiled_src); ++c) {
                auto tile = tiled_src.tile_at(r, c);

                auto key = std::make_tuple(tile.width, tile.height);
                if (cache.find(key) == cache.end()) {
                    int32_t f_dx = std::round(foffset * tile.width );
                    int32_t f_dy = std::round(foffset * tile.height);
                    int32_t b_dx = std::max(1.0, std::round(boffset * tile.width ));
                    int32_t b_dy = std::max(1.0, std::round(boffset * tile.height));
                    cv::Mat_<uint8_t> mask(tile.height + 2 * b_dy, tile.width + 2 * b_dx);

                    auto&& [ dx, dy, selection, fidxs, bidxs ] = cache[key];
                    dx = b_dx;
                    dy = b_dy;

                    selection = cv::Rect(
                        b_dx * 2
                      , b_dy * 2
                      , tile.width  - b_dx * 4
                      , tile.height - b_dy * 4
                    );
                    mask = 1;
                    mask(selection) = 0;
                    cv::findNonZero(mask, bidxs);

                    selection = cv::Rect(
                        b_dx + f_dx
                      , b_dy + f_dy
                      , tile.width  - (b_dx + f_dx) * 2
                      , tile.height - (b_dx + f_dx) * 2
                    );
                    mask = 0;
                    mask(selection) = 1;
                    cv::findNonZero(mask, fidxs);
                }
                auto&& [ dx, dy, selection, fidxs, bidxs ] = cache[key];
                tile.x -= dx;
                tile.y -= dy;
                tile.width  += dx * 2;
                tile.height += dy * 2;

                cv::Mat_<uint16_t> patch = matx(tile);
                auto fgval = partial_percentile(patch, frank, fidxs);
                auto bgval = partial_percentile(patch, brank, bidxs);
                auto stdev = 0.0;
                for (auto&& point: fidxs)
                    stdev = static_cast<double>(patch(point)) * patch(point);
                stdev /= fidxs.size();
                stdev -= fgval * fgval;
                stdev = std::sqrt(stdev);

                res.mean  (r, c) = std::max(1.0, fgval - bgval);
                res.stddev(r, c) = stdev;
                res.cv    (r, c) = stdev / fgval;
                res.bg    (r, c) = bgval;
                res.num   (r, c) = fidxs.size();

                tile.x += selection.x;
                tile.y += selection.y;
                tile.width  = selection.width;
                tile.height = selection.height;
                if( tile_replace )
                    tiled_src.tile_at(r, c) = tile;
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
