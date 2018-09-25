#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/grid_raw_img.hpp>
#include <range/v3/all.hpp>
#include <Nucleona/tuple.hpp>
#include <iostream>
namespace chipimgproc::algo{

struct MatChunk
{

    template<class MAT>
    auto operator() ( MAT& mat, std::size_t ch_size_x, std::size_t ch_size_y ) const {
        auto x_anchors = ranges::view::ints(0, cols(mat)) 
            | ranges::view::chunk(ch_size_x)
            | ranges::view::transform([](auto&& ch){
                return std::make_tuple(*ch.begin(), ranges::distance(ch));
            })
        ;
        auto y_anchors = ranges::view::ints(0, rows(mat)) 
            | ranges::view::chunk(ch_size_y)
            | ranges::view::transform([](auto&& ch){
                return std::make_tuple(*ch.begin(), ranges::distance(ch));
            })
        ;
        return ranges::view::for_each(y_anchors, [x_anchors, &mat](auto&& y_a){
            auto&& [y, y_size] = y_a;
            return ranges::view::for_each(x_anchors, [y, y_size, &mat](auto&& x_a){
                auto&& [x, x_size] = x_a;
                return ranges::yield(
                    std::make_tuple(
                        x, y,
                        get_roi( mat, cv::Rect(
                            x, y,
                            x_size, y_size
                        ))
                    )
                );
            });
        });
    }
};
}
