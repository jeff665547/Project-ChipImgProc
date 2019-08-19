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
#include "mid_seg.hpp"
namespace chipimgproc::margin{

template<class FLOAT>
struct Max {

    auto tile_max(
          const cv::Mat& src
        , const cv::Rect& t
    ) const {
        cv::Mat_<std::uint16_t> src_ = src;
        auto cell_data = src_(t);
        stat::Cell<FLOAT> res;
        res.cv = std::numeric_limits<FLOAT>::max();
        res.stddev = std::numeric_limits<FLOAT>::max();
        res.num = 1;

        std::uint16_t max_value = 0;
        for(int r = 0; r < cell_data.rows; r++) {
            for(int c = 0; c < cell_data.cols; c ++) {
                auto value = cell_data(r, c);
                max_value = std::max(value, max_value);
            }
        }
        res.mean = max_value;
        return res;
    }
    template<class GLID>
    auto operator()( 
          TiledMat<GLID>&           tiled_src
        , bool                      tile_replace        = true
        , const std::function<
            void(const cv::Mat&)
          >&                        v_result            = nullptr
    ) const {
        MidSeg<FLOAT> mid_seg;
        mid_seg(tiled_src, 0.8, true);
        stat::Mats<FLOAT> res(rows(tiled_src), cols(tiled_src));
        auto& tiles = tiled_src.get_tiles();
        for( int y = 0; y < rows(tiled_src); y ++ ) {
            for ( int x = 0; x < cols(tiled_src); x ++ ) {
                auto t = tiled_src.tile_at(y, x);
                auto pt_data = tile_max(
                    tiled_src.get_cali_img(), t
                );
                res.mean   (y, x) = pt_data.mean;
                res.stddev (y, x) = pt_data.stddev;
                res.cv     (y, x) = pt_data.cv;
                res.num    (y, x) = pt_data.num;
                if( tile_replace )
                    tiled_src.tile_at(y, x) = t;
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
    }

};

}