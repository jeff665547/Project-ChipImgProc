#pragma once
#include <ChipImgProc/utils.h>
#include "layout.hpp"
#include "detection/mk_region.hpp"

namespace chipimgproc::marker {

constexpr struct ROIAppend {
    auto operator()(
        const cv::Mat& src, 
        const Layout& layout, 
        const std::vector<detection::MKRegion>& mk_regs,
        std::ostream& log = nucleona::stream::null_out
    ) const {
        int height = 0;
        int width = 0;
        for(int i = 0; i < layout.mk_map.rows; i++ ) {
            int local_width = 0;
            int max_height = 0;
            for( int j = 0; j < layout.mk_map.cols; j ++) {
                auto& mk_r = mk_regs.at(layout.mk_map(i, j));
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
        log << "[ROIAppend] height: " << height << std::endl;
        log << "[ROIAppend] width: "  << width  << std::endl;

        cv::Mat res(height, width, src.type());
        cv::Rect curr_window(0, 0, 0, 0);
        for(int i = 0; i < layout.mk_map.rows; i++ ) {
            int max_height = 0;
            curr_window.x = 0;
            for( int j = 0; j < layout.mk_map.cols; j ++) {
                auto& mk_r = mk_regs.at(layout.mk_map(i, j));
                log << "[ROIAppend] mk_r: " << mk_r << std::endl;
                auto mat = src(mk_r);
                curr_window.width  = mk_r.width;
                curr_window.height = mk_r.height;
                log << "[ROIAppend] curr_window: " << curr_window << std::endl;
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
} roi_append;

}