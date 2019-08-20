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
#include <ChipImgProc/margin/mid_seg.hpp>
namespace chipimgproc::margin{

template<class FLOAT>
struct Percentile {

    auto tile_percentile(
          const cv::Mat& src
        , const cv::Rect& t
        , FLOAT percentage
    ) const {
        auto trim_rate = (1.0 - percentage) / 2;
        auto cell_data = src(t);
        std::vector<std::uint16_t> buf(
            cell_data.begin<std::uint16_t>(), 
            cell_data.end<std::uint16_t>() 
        );
        std::sort(buf.begin(), buf.end());
        std::size_t begin_pos = std::round(trim_rate * buf.size());
        std::size_t capture_size = std::round(percentage * buf.size());

        stat::Cell<FLOAT> res;
        res.cv = std::numeric_limits<FLOAT>::max();
        res.stddev = std::numeric_limits<FLOAT>::max();
        res.num = capture_size;

        FLOAT sum = 0;
        for(std::size_t i = begin_pos; i < begin_pos + capture_size; i ++ ) {
            sum += buf.at(i);
        }
        res.mean = sum / res.num;
        return res;
    }
    template<class GLID>
    auto operator()( 
          TiledMat<GLID>&           tiled_src
        , float                     percentage
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
                auto pt_data = tile_percentile(
                    tiled_src.get_cali_img(), t, percentage
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