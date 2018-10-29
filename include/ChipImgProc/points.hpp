/**
 *  @file    ChipImgProc/points.hpp
 *  @author  Chia-Hua Chang
 */
#pragma once
#include <Nucleona/range.hpp>
#include <ChipImgProc/utils.h>

namespace chipimgproc{ 
/**
 *  @brief Spen the row and column and generate the point list.
 *  @detail For example: call make_points<Point>(2,2) will generate a list [(0,0), (1,0), (0,1), (1,1)]
 *  @tparam P The return points type.
 *  @param rows The row number of points.
 *  @param cols The column number of points.
 *  @return A range of points.
 */
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
/**
 *  @brief Same as make_points(rows, cols), but use the mat row and column as parameter. 
 *  @tparam P The return point type.
 *  @param mat Input matrix.
 *  @return A range of points.
 */
template<class P, class MAT>
decltype(auto) make_points(MAT&& mat) {
    return make_points<P>(rows(mat), cols(mat));
}

}