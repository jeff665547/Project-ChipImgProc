#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/stat/mats.hpp>

namespace chipimgproc { namespace margin{

template<class FLOAT>
struct OnlyStat
{
    template<class GLID>
    auto operator()( 
          TiledMat<GLID>&           tiled_src
        , const std::function<
            void(const cv::Mat&)
          >&                        v_result            = nullptr
    )
    {
        auto ts_rows = rows(tiled_src);
        auto ts_cols = cols(tiled_src);
        stat::Mats<FLOAT> res(ts_rows, ts_cols);
        for( int y = 0; y < ts_rows; y ++ ) {
            for( int x = 0; x < ts_cols; x ++ ) {
                cv::Rect tile( tiled_src.tile_at(y, x) );
                auto cell = stat::Cell<FLOAT>::make(
                    tiled_src.get_cali_img()(tile)
                );
                res.mean   (y, x) = cell.mean;
                res.stddev (y, x) = cell.stddev;
                res.cv     (y, x) = cell.cv;
                res.num    (y, x) = cell.num;
            }
        }

        // draw gridding result
        if(v_result)
            tiled_src.view(v_result);

        return res;
    }
};

}}