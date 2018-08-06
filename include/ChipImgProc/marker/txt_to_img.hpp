#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <string>
#include <istream>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc{ namespace marker{

struct TxtToImg {
    cv::Mat_<std::uint8_t> operator()(
        const cv::Mat_<std::uint8_t>& mat,
        float cell_r_px,
        float cell_c_px,
        float border_px
    ) {
        auto r_cell_bd = cell_r_px + border_px;
        auto c_cell_bd = cell_c_px + border_px;
        auto img_row = 
            border_px + 
            r_cell_bd * mat.rows;
        auto img_col = 
            border_px + 
            c_cell_bd * mat.cols;

        cv::Mat_<std::uint8_t> img = cv::Mat_<std::uint8_t>::zeros(
            (int)std::round(img_row), 
            (int)std::round(img_col)
        );
        int i = 0;
        for( auto r = border_px; r < img.rows; r += r_cell_bd ) {
            int j = 0;
            for( auto c = border_px; c < img.cols; c += c_cell_bd ) {
                if( mat(i,j) == 255 ) {
                    cv::Rect cell(
                        (int)std::round(c), 
                        (int)std::round(r), 
                        (int)std::round(cell_c_px), 
                        (int)std::round(cell_r_px)
                    );
                    cv::rectangle(img, cell, 255, CV_FILLED);
                }
                j ++;
            }
            i ++;
        }
        return img;
    }
};

}}