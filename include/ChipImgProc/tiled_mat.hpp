#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range/irange.hpp>
namespace chipimgproc{

template<
    class TID = std::uint32_t, 
    class GLID = std::uint16_t
>
struct TiledMat : public cv::Mat_<TID>
{
    template<class GRID_RES>
    static TiledMat make_from_grid_res(GRID_RES&& grid_res) {
        // TODO: consider forward parameter
        namespace nr = nucleona::range;
        if(
            grid_res.feature_rows * (std::uint32_t)grid_res.feature_cols 
            != grid_res.tiles.size()
        ) {
            throw std::runtime_error("BUG: grid tile number and feature cols * rows not matched");
        }
        TiledMat tm(
            grid_res.feature_rows,
            grid_res.feature_cols
        );
        for( auto r : nr::irange_0(grid_res.feature_rows) ) {
            for( auto c : nr::irange_0(grid_res.feature_cols) ) {
                auto tid = (r * (std::uint32_t)grid_res.feature_cols) + c;
                tm(c, r) = tid;
            }
        }
        tm.cali_img_ = grid_res.rot_cali_res;
        tm.tiles_ = grid_res.tiles;
        tm.gl_x_.reserve(grid_res.gl_x);
        for(auto&& v : grid_res.gl_x ) {
            tm.gl_x_.push_back(v);
        }
        tm.gl_y_.reserve(grid_res.gl_y);
        for(auto&& v : grid_res.gl_y ) {
            tm.gl_y_.push_back(v);
        }
        
        return tm;
    }
private:
    cv::Mat cali_img_;
    std::vector<cv::Rect> tiles_;
    std::vector<GLID> gl_x_, gl_y_;
};
}