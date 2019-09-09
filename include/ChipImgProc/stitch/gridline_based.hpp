/**
 * @file gridline_based.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::stitch::GridlineBased.
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/multi_tiled_mat.hpp>
#include <ChipImgProc/logger.hpp>
namespace chipimgproc{ namespace stitch{ 
/**
 * @brief Grid line based stitching algorithm.
 * @details This algorithm stitch the FOV images based
 *   on the grid line information, client should provide 
 *   chipimgproc::MultiTiledMat object which contains cell
 *   level stitching point and FOV grid lines then the 
 *   stitching process may performed.
 *   The overlapping region process is direct cover the 
 *   current processed image iteratively.
 */
class GridlineBased {

    template<class GLID, class SHIFT>
    std::vector<GLID> gridline_merge(
        const std::vector<GLID>& gl0,
        SHIFT shift, const std::vector<GLID>& gl1 
    ) const {
        std::vector<GLID> res = gl0;
        GLID shift_px = res.at(shift) - gl1.at(0);
        GLID j = 0;
        for( GLID i = shift; i < gl0.size() && j < gl1.size(); i ++ ) {
            res.at(i) = ( res.at(i) + gl1.at(j) + shift_px) / 2;
            j ++;
        }
        for( ; j < gl1.size(); j ++ ) {
            res.push_back(gl1.at(j) + shift_px);
        }
        return res;
    }
    template<class GLID>
    void merge(
        GridRawImg<GLID>& m, 
        const GridRawImg<GLID>& __raw_img, 
        const cv::Point& st_ps
    ) const {
        auto raw_img = __raw_img.clean_border();
        log.trace("cell level stitch point: ({},{})", st_ps.x, st_ps.y);
        if(m.empty()) {
            m.mat()  = raw_img.mat();
            m.gl_x() = raw_img.gl_x();
            m.gl_y() = raw_img.gl_y();
        } else {
            m.gl_x() = gridline_merge(m.gl_x(), st_ps.x, raw_img.gl_x());
            m.gl_y() = gridline_merge(m.gl_y(), st_ps.y, raw_img.gl_y());

            cv::Point st_ps_px(
                m.gl_x().at(st_ps.x) - raw_img.gl_x().at(0), 
                m.gl_y().at(st_ps.y) - raw_img.gl_y().at(0)
            );

            cv::Mat res(
                std::max(st_ps_px.y + raw_img.mat().rows, m.mat().rows), 
                std::max(st_ps_px.x + raw_img.mat().cols, m.mat().cols),
                m.mat().type()
            );

            cv::Rect m_roi      (0, 0, m.mat().cols, m.mat().rows);
            cv::Rect raw_img_roi(
                st_ps_px.x, st_ps_px.y, 
                raw_img.mat().cols, raw_img.mat().rows
            );
            m.mat().copyTo(res(m_roi));
            raw_img.mat().copyTo(res(raw_img_roi));
            m.mat().release();
            m.mat() = res;
        }
    }
public:
    /**
     * @brief Call operator of the stitcher.
     * @details @copydetails chipimgproc::stitch::GridlineBased
     * 
     * @tparam FLOAT Deduced, the float point type of chipimgproc::MultiTiledMat.
     * @tparam GLID Deduced, the grid line storage type chipimgproc::MultiTiledMat.
     * 
     * @param mat The multiple tiled matrix, 
     *   which contains the grid lines and cell level stitching points.
     * 
     * @return GridRawImg<GLID> The stitched image with grid line.
     */
    template<class FLOAT, class GLID>
    GridRawImg<GLID> operator() ( const MultiTiledMat<FLOAT, GLID>& mat ) const {
        GridRawImg<GLID> res;
        for( int i = 0; i < mat.get_fov_rows(); i ++ ) {
            for( int j = 0; j < mat.get_fov_cols(); j ++ ) {
                auto& fov_img = mat.get_fov_img(j, i);
                merge(
                    res,
                    fov_img,
                    mat.cell_level_stitch_point(j, i)
                );
            }
        }
        return res;
    }
};


}}