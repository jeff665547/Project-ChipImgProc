#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <string>
#include <istream>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc{ namespace marker{

struct TxtToImg {
    auto operator()(
        const cv::Mat_<std::uint8_t>& mat,
        const cv::Mat_<std::uint8_t>& mask_cl,
        float cell_r_px,
        float cell_c_px,
        float border_px
    ) {
        // auto ext = cell_r_px * 0.2;
        // cell_r_px += ext;
        // cell_c_px += ext;
        // border_px -= ext;
        std::cout << "mask_cl: " << std::endl;
        std::cout << mask_cl << std::endl;
        cell_r_px *= 16;
        cell_c_px *= 16;
        border_px *= 16;

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
        cv::Mat_<std::uint8_t> mask = cv::Mat_<std::uint8_t>::zeros(
            (int)std::round(img_row), 
            (int)std::round(img_col)
        );
        int i = 0;
        for( auto r = border_px; r < img.rows; r += r_cell_bd ) {
            int j = 0;
            for( auto c = border_px; c < img.cols; c += c_cell_bd ) {
                cv::Rect cell(
                    (int)std::round(c), 
                    (int)std::round(r), 
                    (int)std::round(cell_c_px), 
                    (int)std::round(cell_r_px)
                );
                cv::rectangle(img, cell, mat(i,j), CV_FILLED);
                cv::rectangle(mask, cell, mask_cl(i, j), CV_FILLED);
                j ++;
            }
            i ++;
        }
        cv::Mat img_tmp; 
        cv::Mat mask_tmp;
        for( int i = 0; i < 4; i ++ ) {
            cv::pyrDown(img, img_tmp);
            cv::pyrDown(mask, mask_tmp);
            img = img_tmp.clone();
            mask = mask_tmp.clone();
        }
        return nucleona::make_tuple(
            img_tmp.clone(), 
            mask_tmp.clone()
        );
    }
};

}}