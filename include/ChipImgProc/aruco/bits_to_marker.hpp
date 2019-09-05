/**
 * @file bits_to_marker.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::aruco::BitsToMarker
 * 
 */
#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <Nucleona/language.hpp>
#include <bitset>
namespace chipimgproc::aruco {
/**
 * @brief Convert ArUco bits map to marker image
 * 
 */
constexpr struct BitsToMarker {
    /**
     * @brief Call operator, given ArUco code and one side bits number and generate marker image.
     * 
     * @param code              ArUco code encoded in integer.
     *                          The lowest bit is image left top bit.
     * @param side_bits_length  Bits number of marker side.
     * @param bit_width         Bit width in pixel.
     * @param pyramid_level     Result scale down 2^pyramid_level times. 
     * @return cv::Mat          The ArUco marker pattern. Coding bit is 255, otherwise 0.
     */
    cv::Mat operator()(
        std::uint64_t   code,
        std::int32_t    side_bits_length,
        double          bit_width,
        std::int32_t    pyramid_level
    ) const {
        // std::int32_t bit_width = d_bit_width
        bit_width *= 16;
        int img_side_length = std::round(side_bits_length * bit_width);

        cv::Mat_<std::uint8_t> img = cv::Mat_<std::uint8_t>::zeros(
            img_side_length, img_side_length
        );
        cv::Rect img_bound(0, 0, img.cols, img.rows);
        std::uint64_t mask = 0x01;
        // int i = 0;
        for(int i = 0; i < side_bits_length; i ++ ) { // row
            for(int j = 0; j < side_bits_length; j ++ ) { // col
                int c = std::round(j * bit_width);
                int r = std::round(i * bit_width);
                cv::Rect cell(
                    c, 
                    r, 
                    std::round(bit_width), 
                    std::round(bit_width)
                );
                cell = cell & img_bound;
                auto bit = code & mask;
                auto cell_cont = bit == 0 ? 0 : 255;
                cv::rectangle(img, cell, cell_cont, cv::FILLED);
                code = code >> 1;
            }
        }
        cv::Mat img_tmp; 
        for( int i = 0; i < (pyramid_level + 4); i ++ ) {
            cv::pyrDown(img, img_tmp);
            img = img_tmp.clone();
        }
        return img_tmp;
    }

} bits_to_marker;

}