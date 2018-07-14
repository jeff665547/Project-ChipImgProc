#pragma once
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <vector>
namespace chipimgproc{ namespace marker{ namespace detection{

struct ContourRefine {
    cv::Mat_<std::uint8_t> binarize(const cv::Mat& src ) const {
        cv::Mat_<std::uint8_t> tmp;
        src.convertTo(tmp, CV_8U, 0.00390625);
        cv::Mat_<std::uint8_t> bin;
        cv::threshold(tmp, bin, 150, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        return bin;
        // return chipimgproc::binarize(tmp);
    }
    std::vector<cv::Rect> find_contours(const cv::Mat_<std::uint8_t>& bin_src) const {
        std::vector<std::vector<cv::Point>> contours;
        std::vector<cv::Rect> res;
        cv::findContours(
            bin_src, contours,
            CV_RETR_LIST,
            CV_CHAIN_APPROX_SIMPLE
        );
        for( auto& ps : contours ) {
            res.push_back(bound_rect(ps));
        }
        return res;
    }
    auto contour_consensus( std::vector<cv::Rect>& cts ) const {
        int w_sum(0), h_sum(0);
        int x_min = std::numeric_limits<int>::max();
        int y_min = std::numeric_limits<int>::max();
        for( auto&& r : cts ) {
            w_sum += r.width;
            h_sum += r.height;
            if( x_min > r.x ) x_min = r.x;
            if( y_min > r.y ) y_min = r.y;
        }
        auto w = w_sum / cts.size();
        auto h = h_sum / cts.size();
        return std::make_tuple(w, h, x_min, y_min);
    }
    void operator()( 
        const cv::Mat&              src, 
        std::vector<MKRegion>&      mk_regs,
        const Layout&               layout
    ) const {
        auto v_src = viewable(src);

        auto bin_src = binarize(src);
        cv::imwrite("debug_bin.tiff", bin_src);
        for( auto&& mk_r : mk_regs ) {
            cv::rectangle(v_src, mk_r, 32767, 3);
            auto mk_img = bin_src(mk_r);
            auto cts = find_contours(mk_img);
            for(auto&& ct : cts ) {
                ct.x += mk_r.x;
                ct.y += mk_r.y;
                cv::rectangle(v_src, ct, 32767, 3);
            }
            std::cout << "contour num: " << cts.size() << std::endl;
            // TODO: QC the contour, 
            // now assume the contour doesn't have physical break.

            auto[w, h, x_min, y_min] = contour_consensus(cts);
            // TODO: find the related initial point of now assume LT (-2, -2)

            mk_r.x = x_min - ( (w+1) * 2 );
            mk_r.y = y_min - ( (h+1) * 2 );
        }
        cv::imwrite("debug_rect.tiff", v_src);
    }
};

}}}