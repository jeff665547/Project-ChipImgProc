#pragma once
#include <ChipImgProc/utils.h>
#include <iostream>
namespace chipimgproc{ namespace stitch{

struct PositionBased {
    PositionBased( int row, int col ) 
    : row_ ( row )
    , col_ ( col )
    {}

    cv::Mat operator()(
        const std::vector<cv::Mat>& imgs,
        const std::vector<cv::Point_<int>>& st_ps
    ) {
        if( imgs.size() != st_ps.size()) {
            throw std::runtime_error(
                std::string("size assertion fail") + 
                __FILE__ + ":" + std::to_string(__LINE__)
            );
        }
        auto w_h = get_full_w_h(imgs, st_ps);
        // std::vector<cv::Mat> layers;
        cv::Mat res(w_h.y + 500, w_h.x + 500, imgs.at(0).type());
        // cv::Mat tmp(w_h.y, w_h.x, imgs.at(0).type());
        // int from_to[2];
        bool first = true;
        cv::Rect last_region;
        for( int i = 0; i < imgs.size(); i ++ ) {
            // cv::Mat layer( w_h.y, w_h.x, imgs.at(0).type());
            auto& min_p = st_ps.at(i);
            auto& img_i = imgs.at(i);
            cv::Rect region(min_p.x, min_p.y, img_i.cols, img_i.rows);
            if(!first) {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                auto inter = last_region & region;
                std::cout << "inter: " << inter << std::endl;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                cv::Rect base_region(
                    std::max(inter.x - 50, 0),
                    std::max(inter.y - 50, 0),
                    inter.width  + 100,
                    inter.height + 100
                );
                std::cout << base_region << std::endl;
                cv::Mat base = res(base_region);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                cv::Mat obj = img_i(cv::Rect(
                    inter.x - region.x, 
                    inter.y - region.y, 
                    inter.width, 
                    inter.height
                )); // TODO:
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                cv::Mat scores;
                cv::matchTemplate(base, obj, scores, cv::TM_CCORR_NORMED);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;

                double val;
                cv::Point loc;
                cv::minMaxLoc(scores, nullptr, &val, nullptr, &loc);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                region.x = base_region.x + loc.x;
                region.y = base_region.y + loc.y;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            }
            img_i.copyTo(res(region));
            
            // res.copyTo(tmp);
            // img_i.copyTo(tmp(region));
            // from_to[0] = i % 3;
            // from_to[1] = (i + 2) % 3;
            // cv::mixChannels(&tmp, 1, &res, 1, from_to, 1);
            // img_i.copyTo(layer(region));
            // layers.push_back(layer);
            last_region = region;
            first = false;
        }
        // cv::merge(layers.data(), layers.size(), res);
        return res;
    }
private:
    cv::Point_<int> get_full_w_h( 
        const std::vector<cv::Mat>& imgs, 
        const std::vector<cv::Point_<int>>& st_ps
    ) {
        cv::Point_<int> min(
            std::numeric_limits<int>::max(),
            std::numeric_limits<int>::max()
        ); 
        cv::Point_<int> max(
            std::numeric_limits<int>::min(),
            std::numeric_limits<int>::min()
        );
        for ( int i = 0; i < imgs.size(); i ++ ) {
            auto curr_min = st_ps.at(i);
            cv::Point_<int> curr_max( 
                curr_min.x + imgs.at(i).cols,
                curr_min.y + imgs.at(i).rows
            );
            if( min.x > curr_min.x ) min.x = curr_min.x;
            if( min.y > curr_min.y ) min.y = curr_min.y;
            if( max.x < curr_max.x ) max.x = curr_max.x;
            if( max.y < curr_max.y ) max.y = curr_max.y;
        }
        return cv::Point_<int>( 
            max.x - min.x,
            max.y - min.y
        );
    }
    int row_;
    int col_;
};

}}