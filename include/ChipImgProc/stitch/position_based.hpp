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
            cv::Rect region(min_p.x + 50, min_p.y + 50, img_i.cols, img_i.rows);
            if(!first) {
                std::cout << "last_region: " << last_region << std::endl;
                std::cout << "region: " << region << std::endl;
                auto inter = last_region & region;
                std::cout << "inter: " << inter << std::endl;
                cv::Rect base_region(inter);
                cv::Rect obj_region(
                    inter.x - region.x + 50, 
                    inter.y - region.y + 50, 
                    inter.width - 100, 
                    inter.height - 100
                );
                std::cout << "base_region: " << base_region << std::endl;
                std::cout << "obj_region: " << obj_region << std::endl;
                cv::Mat base = res(base_region);
                cv::Mat obj = img_i(obj_region); // TODO:
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                cv::Mat scores(base.rows - obj.rows + 1, base.cols - obj.cols + 1, CV_32F);
                cv::matchTemplate(base, obj, scores, cv::TM_CCORR_NORMED);
                cv::imwrite("obj_" + std::to_string(i) + ".tiff", obj);
                cv::imwrite("base_" + std::to_string(i) + ".tiff", base);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;

                double val;
                cv::Point loc;
                cv::minMaxLoc(scores, nullptr, &val, nullptr, &loc);
                std::cout << "loc: " << loc << std::endl;
                std::cout << "val: " << val << std::endl;
                // cv::imwrite("score_" + std::to_string(i) + ".tiff", scores);
                cv::imwrite(
                    "obj_add_base_" + std::to_string(i) + ".tiff", 
                    add(base(cv::Rect(loc.x, loc.y, obj.cols, obj.rows)), obj)
                );

                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                region.x = base_region.x + loc.x - obj_region.x;
                region.y = base_region.y + loc.y - obj_region.y;
                std::cout << "region: " << region << std::endl;
            }
            basic_stitch(img_i, res, region, i);
            
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
    cv::Mat score_img( const cv::Mat& s ) {
        
    }
    cv::Mat add(const cv::Mat& a, const cv::Mat& b ) {
        cv::Mat res;
        cv::addWeighted(a, 0.5, b, 0.5, 0, res);
        return res;
    }
    void basic_stitch( 
        const cv::Mat& obj, 
        cv::Mat& base, 
        const cv::Rect& region,
        int i 
    ) {
        // int from_to[] = { i % 3, (i + 1) % 3};
        cv::Mat tmp(base.rows, base.cols, base.type());
        obj.copyTo(tmp(region));
        // cv::mixChannels(&tmp, 1, &base, 1, from_to, 1);
        // cv::imwrite("mixed_" + std::to_string(i) + ".tiff", base);
        // obj.copyTo(base(region));
        cv::Mat res;
        cv::Mat overlap;
        cv::addWeighted(base(region), 0.5, tmp(region), 0.5, 0, overlap);
        cv::addWeighted(base, 1, tmp, 1, 0, res);
        // cv::add(base, tmp, res);
        overlap.copyTo(res(region));
        base = res;

    }
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