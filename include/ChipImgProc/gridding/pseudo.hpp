/**
 * @file pseudo.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::gridding::Pseudo
 * 
 */
#pragma once
#include "result.hpp"
#include "utils.hpp"
#include <functional>
#include <ChipImgProc/utils.h>
namespace chipimgproc::gridding{

/**
 * @brief Pesudo gridding. Actually do nothing, 
 *        just generate the general gridding result (chipimgproc::gridding::Result)
 *        from the grid lines.
 * 
 */
struct Pseudo {
    /**
     * @brief Call operator.
     * 
     * @param gl_x      Grid line points along the x direction.
     * @param gl_y      Grid line points along the y direction.
     * @param v_result  Debug view, show the gridding result.
     * @param in_src    Debug image, for show the gridding result.
     * @return Result   General gridding result.
     */
    Result operator()(
        const std::vector<double>&        gl_x,
        const std::vector<double>&        gl_y, 
        const ViewerCallback&             v_result    = nullptr,
        cv::Mat*                          in_src      = nullptr
    ) const {
        Result res;
        res.feature_rows = gl_y.size() - 1;
        res.feature_cols = gl_x.size() - 1;
        res.tiles        = gridline_to_tiles(gl_x, gl_y);
        res.gl_x         = gl_x;
        res.gl_y         = gl_y;
        if(v_result && !in_src->empty()) {
            cv::Mat_<std::uint16_t> debug_img = viewable(*in_src);
            auto color = 65536/2;
            for (auto tile: res.tiles)
            {
                tile.width  += 1;
                tile.height += 1;
                cv::rectangle(debug_img, tile, color);
            }
            v_result(debug_img);

        }
        return res;
    }
};

}