#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc::gridding {
constexpr struct GridlineToTiles {
    std::vector<cv::Rect> operator()(
        const std::vector<double>& x_grid_anchor,
        const std::vector<double>& y_grid_anchor
    ) const {
        std::vector<cv::Rect> tiles;
        for(std::size_t i = 1; i < y_grid_anchor.size(); i ++ ) {
            for (std::size_t j = 1; j < x_grid_anchor.size(); j ++ ) {
                auto w = x_grid_anchor.at(j) - x_grid_anchor.at(j-1);
                auto h = y_grid_anchor.at(i) - y_grid_anchor.at(i-1);
                tiles.push_back(
                    cv::Rect(
                        (int)std::round(x_grid_anchor.at(j-1)),
                        (int)std::round(y_grid_anchor.at(i-1)),
                        (int)std::round(w), 
                        (int)std::round(h)
                    )
                );
            }
        }
        return tiles;
    }
} gridline_to_tiles;
}