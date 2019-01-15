#pragma once
#include <vector>
#include <ChipImgProc/marker/detection/mk_region.hpp>
namespace chipimgproc::marker::detection {

constexpr struct Infer {
    template<class T>
    auto operator() ( 
        const cv::Mat_<T>&      src, 
        std::vector<MKRegion>&  mk_regs,
        const ViewerCallback&   v_marker  = nullptr 
    ) const {
        std::vector<double> x_anchor;
        std::vector<double> y_anchor;

        auto x_group_p = MKRegion::x_group_points(mk_regs);
        for(auto& p : x_group_p) {
            auto& vec = p.second;
            std::size_t x_sum = 0;
            int num = 0;
            for(auto& mk : vec ) {
                x_sum += mk.x;
                num ++;
            }
            auto mean = (double)x_sum / num;
            x_anchor.push_back(mean);
        }

        auto y_group_p = MKRegion::y_group_points(mk_regs);
        for(auto& p : y_group_p) {
            auto& vec = p.second;
            std::size_t y_sum = 0;
            int num = 0;
            for(auto& mk : vec ) {
                y_sum += mk.y;
                num ++;
            }
            auto mean = (double)y_sum / num;
            y_anchor.push_back(mean);
        }

        double width = 0;
        double height = 0;
        for( auto&& mk : mk_regs) {
            width += mk.width;
            height += mk.height;
        }
        width /= mk_regs.size();
        height /= mk_regs.size();

        std::vector<MKRegion> new_mk_regs;
        for(int y_i = 0; y_i < y_anchor.size(); y_i ++ ) {
            for( int x_i = 0; x_i < x_anchor.size(); x_i ++ ) {
                MKRegion mkr; // TODO: score
                mkr.x_i = x_i;
                mkr.y_i = y_i;
                mkr.x = x_anchor[x_i];
                mkr.y = y_anchor[y_i];
                mkr.width = width;
                mkr.height = height;
                new_mk_regs.push_back(mkr);
            }
        }
        auto view = norm_u8(src, 0, 0); 
        if(v_marker) {
            for(auto& mk_r : new_mk_regs) {
                cv::rectangle(view, mk_r, 128, 1);
            }
            v_marker(view);
        }
        return new_mk_regs;
    } 
} infer;

}