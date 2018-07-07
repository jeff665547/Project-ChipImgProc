#pragma once
#include <Nucleona/range.hpp>
#include <ChipImgProc/utils.h>

namespace chipimgproc{ 

template<class P>
decltype(auto) make_points(int rows, int cols) {
    return nucleona::range::transform(
        nucleona::range::irange_0(rows * cols),
        [rows, cols, c = 0, r = 0](auto i) mutable {
            P p{c, r};
            if( c < cols) c++;
            else {
                c = 0;
                r++;
            }
            return p;
        }
    );
}

template<class P, class MAT>
decltype(auto) make_points(MAT&& mat) {
    return make_points<P>(rows(mat), cols(mat));
}

}