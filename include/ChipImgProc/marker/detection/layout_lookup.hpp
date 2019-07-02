#pragma once
#include "mk_region.hpp"
#include <ChipImgProc/marker/layout.hpp>
namespace chipimgproc::marker::detection {

constexpr struct LayoutLookup {
    template<class GLRange>
    std::vector<MKRegion> operator()(
        GLRange&& gl_x,
        GLRange&& gl_y,
        const Layout& layout
    ) const {
        std::vector<MKRegion> res;
        res.reserve(layout.mks.size());
        for(int r = 0; r < layout.mk_map.rows; r ++ ) {
            for(int c = 0; c < layout.mk_map.cols; c ++ ) {
                auto& mk_des = layout.get_marker_des(r, c);
                auto& mk = mk_des.get_std_mk(MatUnit::CELL);
                auto& pos = mk_des.get_pos(MatUnit::CELL);
                MKRegion reg;
                reg.x_i = c;
                reg.y_i = r;
                reg.x = gl_x[pos.x];
                reg.y = gl_y[pos.y];
                auto x_end = gl_x[pos.x + mk.cols];
                auto y_end = gl_y[pos.y + mk.rows];
                reg.width = x_end - reg.x;
                reg.height = y_end - reg.y;
                res.emplace_back(std::move(reg));
            }
        }
        return res;
    }
} layout_lookup;

}