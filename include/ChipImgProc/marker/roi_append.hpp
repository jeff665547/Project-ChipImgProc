#pragma once
#include <ChipImgProc/utils.h>
#include "layout.hpp"
#include "detection/mk_region.hpp"
#include <ChipImgProc/logger.hpp>

namespace chipimgproc::marker {

constexpr struct ROIAppend {
    auto operator()(
        const cv::Mat& src, 
        const cv::Mat_<std::int16_t>& mk_map,
        const std::vector<detection::MKRegion>& mk_regs,
        std::ostream& __log = nucleona::stream::null_out
    ) const {
        int height = 0;
        int width = 0;
        for(int i = 0; i < mk_map.rows; i++ ) {
            int local_width = 0;
            int max_height = 0;
            for( int j = 0; j < mk_map.cols; j ++) {
                auto& mk_r = mk_regs.at(mk_map(i, j));
                local_width += mk_r.width;
                if( mk_r.height > max_height) {
                    max_height = mk_r.height;
                }
            }
            if( local_width > width ) {
                width = local_width;
            }
            height += max_height;
        } 
        log.debug("[ROIAppend] height: {}", height);
        log.debug("[ROIAppend] width:  {}", width );

        cv::Mat res(height, width, src.type());
        cv::Rect curr_window(0, 0, 0, 0);
        for(int i = 0; i < mk_map.rows; i++ ) {
            int max_height = 0;
            curr_window.x = 0;
            for( int j = 0; j < mk_map.cols; j ++) {
                auto& mk_r = mk_regs.at(mk_map(i, j));
                log.debug("[ROIAppend] mk_r: ({},{},{},{})", 
                    mk_r.x, mk_r.y, mk_r.width, mk_r.height
                );
                auto mat = src(mk_r);
                curr_window.width  = mk_r.width;
                curr_window.height = mk_r.height;
                log.debug("[ROIAppend] curr_window: ({},{},{},{})", 
                    curr_window.x, curr_window.y,
                    curr_window.width, curr_window.height
                );
                mat.copyTo(res(curr_window));
                curr_window.x += curr_window.width  ; 
                if(mk_r.height > max_height) {
                    max_height = mk_r.height;
                }
            }
            curr_window.y += max_height;
        }
        return res;
    }
    auto operator()(
        const cv::Mat& src, 
        const Layout& layout,
        const std::vector<detection::MKRegion>& mk_regs,
        std::ostream& log = nucleona::stream::null_out
    ) const {
        return operator()(src, layout.mk_map, mk_regs, log);
    }
} roi_append;

}