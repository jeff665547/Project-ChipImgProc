#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/grid_raw_img.hpp>
#include <range/v3/all.hpp>
#include <Nucleona/tuple.hpp>
#include <iostream>
#include <Nucleona/range.hpp>
namespace chipimgproc::algo{

struct MatChunk
{

    template<class MAT>
    auto operator() ( MAT& mat, std::size_t ch_num_x, std::size_t ch_num_y ) const {
        auto x_anchors = ranges::view::ints(0, cols(mat)) 
            | nucleona::range::segment(ch_num_x) // TODO: start from here
            | ranges::view::transform([](auto&& ch){
                return std::make_tuple(*ch.begin(), ranges::distance(ch));
            })
        ;
        auto y_anchors = ranges::view::ints(0, rows(mat)) 
            | nucleona::range::segment(ch_num_y)
            | ranges::view::transform([](auto&& ch){
                return std::make_tuple(*ch.begin(), ranges::distance(ch));
            })
        ;
        return ranges::view::for_each(y_anchors, [x_anchors, &mat](auto&& y_a) mutable {
            auto& y = std::get<0>(y_a);
            auto& y_size = std::get<1>(y_a);
            return ranges::view::for_each(x_anchors, [y, y_size, &mat](auto&& x_a) mutable {
                auto& x = std::get<0>(x_a);
                auto& x_size = std::get<1>(x_a);
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
    template<class GLID>
    auto operator() (GridRawImg<GLID>& mat, std::size_t ch_num_x, std::size_t ch_num_y) const {
        auto cols = mat.gl_x().size() - 1;
        auto rows = mat.gl_y().size() - 1;
        auto x_anchors = ranges::view::ints((decltype(cols))0, cols) 
            | nucleona::range::segment(ch_num_x)
            | ranges::view::transform([](auto&& ch){
                return std::make_tuple(*ch.begin(), ranges::distance(ch));
            })
        ;
        auto y_anchors = ranges::view::ints((decltype(rows))0, rows) 
            | nucleona::range::segment(ch_num_y)
            | ranges::view::transform([](auto&& ch){
                return std::make_tuple(*ch.begin(), ranges::distance(ch));
            })
        ;
        return ranges::view::for_each(y_anchors, [x_anchors, &mat](auto&& y_a) mutable {
            auto& y = std::get<0>(y_a);
            auto& y_size = std::get<1>(y_a);
            return ranges::view::for_each(x_anchors, [y, y_size, &mat](auto&& x_a) mutable {
                auto& x = std::get<0>(x_a);
                auto& x_size = std::get<1>(x_a);
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
