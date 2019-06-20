#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/stat/mats.hpp>

namespace chipimgproc { namespace margin{

template<class FLOAT>
struct MidSeg
{
    template<class GLID>
    auto operator()( 
          TiledMat<GLID>&           tiled_src
        , float                     mid_rate
        , bool                      tile_replace        = true
        , const std::function<
            void(const cv::Mat&)
          >&                        v_result            = nullptr
    ) const 
    {
        if( mid_rate > 1 ) {
            throw std::runtime_error("mid_rate must <= 1");
        }
        auto ts_rows = rows(tiled_src);
        auto ts_cols = cols(tiled_src);
        stat::Mats<FLOAT> res(ts_rows, ts_cols);
        auto& tiles = tiled_src.get_tiles();
        for( int y = 0; y < ts_rows; y ++ ) {
            for( int x = 0; x < ts_cols; x ++ ) {
                cv::Rect tile( tiled_src.tile_at(y, x) );
                int width  = std::round(tile.width  * mid_rate);
                int height = std::round(tile.height * mid_rate);
                auto x_off = ( tile.width  - width  ) / 2;
                auto y_off = ( tile.height - height ) / 2;

                tile.x += x_off;
                tile.y += y_off;
                tile.width = width;
                tile.height = height;

                auto cell = stat::Cell<FLOAT>::make(
                    tiled_src.get_cali_img()(tile)
                );

                res.mean   (y, x) = cell.mean;
                res.stddev (y, x) = cell.stddev;
                res.cv     (y, x) = cell.cv;
                res.num    (y, x) = cell.num;
                if( tile_replace )
                    tiled_src.tile_at(y, x) = tile;

            }
        }

        // draw gridding result
        if(v_result)
            tiled_src.view(v_result);

        return res;
    }
};

}}