#pragma once
#include <map>
#include <cstdint>
#include <ChipImgProc/utils.h>
#include "dictionary.hpp"
#include "bits_to_marker.hpp"
namespace chipimgproc::aruco {

struct MkImgDict
: public std::map<std::int32_t, cv::Mat> // marker index => image
{
    void reset(
        const Dictionary& dict,
        const std::vector<std::int32_t>& candidates,
        double bit_width,
        std::int32_t side_bits_length,
        std::int32_t pyramid_level
    ) 
    {
        for(auto&& id : candidates) {
            auto code = dict.at(id);
            auto mk_img = bits_to_marker(
                code, side_bits_length, bit_width, pyramid_level
            );
            (*this)[id] = mk_img;
        }
    }
    auto mk_idx_at(
        const std::int32_t& id,
        const cv::Mat& tpl
    ) const {
        auto img = tpl.clone();
        auto mk_img = this->at(id);
        cv::Rect mk_roi(
            (int)std::round(img.cols / 2.0 - mk_img.cols / 2.0),
            (int)std::round(img.rows / 2.0 - mk_img.rows / 2.0),
            mk_img.cols,
            mk_img.rows
        );
        mk_img.copyTo(img(mk_roi));
        return img;
    }
};

}