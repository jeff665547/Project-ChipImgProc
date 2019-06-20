#pragma once
#include "result.hpp"
#include "utils.hpp"
#include <functional>
#include <ChipImgProc/utils.h>
namespace chipimgproc::gridding{

struct Pseudo {
    Result operator()(
        const std::vector<std::uint32_t>& gl_x,
        const std::vector<std::uint32_t>& gl_y, 
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