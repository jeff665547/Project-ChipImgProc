#pragma once
#include <ChipImgProc/utils.h>
#include <iostream>
namespace chipimgproc{ namespace stitch{

struct PositionBased {

    std::vector<cv::Point_<int>> operator()(
        const std::vector<cv::Mat>& imgs,
        const std::vector<cv::Point_<int>>& st_ps,
        int cali_max
    ) {
        if( imgs.size() != st_ps.size()) {
            throw std::runtime_error(
                std::string("size assertion fail") + 
                __FILE__ + ":" + std::to_string(__LINE__)
            );
        }
        std::vector<cv::Point_<int>> cali_st_ps;
        for( int i = 0; i < imgs.size(); i ++ ) {
            auto& min_p = st_ps.at(i);
            auto& img_i = imgs.at(i);
            cv::Rect region(min_p.x + cali_max, min_p.y + cali_max, img_i.cols, img_i.rows);
            if(!cali_st_ps.empty()) {
                std::vector<OverlapRes> possible_cali_p;
                for( int i = 0; i < cali_st_ps.size(); i ++ ) {
                    auto overlap_res(
                        overlap(
                            imgs.at(i), cali_st_ps.at(i), 
                            img_i, cv::Point_<int>(region.x, region.y),
                            cali_max
                        )
                    );
                    possible_cali_p.push_back(overlap_res);
                }
                auto vote_res = vote_cali_rect(possible_cali_p);
                cali_st_ps.emplace_back(vote_res.x, vote_res.y);
                region.x = vote_res.x;
                region.y = vote_res.y;
                // basic_stitch(img_i, res, region, i);
            } else {
                cali_st_ps.emplace_back(region.x, region.y);
                // img_i.copyTo(res(region));
            }
        }
        // auto final_region = get_full_w_h(imgs, cali_st_ps);
        // return res(final_region);
        return cali_st_ps;
    }
private:
    struct OverlapRes {
        cv::Rect cali_inter_on_base;
        cv::Rect cali_b_on_base;
    };
    cv::Rect vote_cali_rect( const std::vector<OverlapRes>& candi_rects ) {
        double sum_weight(0);
        double sum_x(0);
        double sum_y(0);
        double sum_w(0);
        double sum_h(0);
        for(auto&& r : candi_rects ) {
            double weight = r.cali_inter_on_base.area();
            sum_x += ( weight * r.cali_b_on_base.x );
            sum_y += ( weight * r.cali_b_on_base.y );
            sum_w += ( weight * r.cali_b_on_base.width );
            sum_h += ( weight * r.cali_b_on_base.height );
            sum_weight += weight;
        }
        cv::Rect res(
            sum_x / sum_weight,
            sum_y / sum_weight,
            sum_w / sum_weight,
            sum_h / sum_weight
        );
        std::cout << "vote_cali_rect return: " << res << std::endl;
        return res;
    }
    OverlapRes overlap( 
        const cv::Mat& a, 
        const cv::Point_<int>& a_on_base_p,
        const cv::Mat& b, 
        const cv::Point_<int>& b_on_base_p,
        const int& b_shrink
    ) {
        cv::Rect a_on_base(
            a_on_base_p.x,
            a_on_base_p.y,
            a.cols,
            a.rows
        );
        cv::Rect b_on_base(
            b_on_base_p.x,
            b_on_base_p.y,
            b.cols,
            b.rows
        );
        auto inter_on_base( a_on_base & b_on_base );
        if( inter_on_base.empty() ) {
            return {inter_on_base, b_on_base};
        }
        cv::Rect inter_on_a(
            inter_on_base.x - a_on_base.x,
            inter_on_base.y - a_on_base.y,
            inter_on_base.width,
            inter_on_base.height
        );
        cv::Rect inter_on_b_shrinked(
            inter_on_base.x - b_on_base.x + b_shrink,
            inter_on_base.y - b_on_base.y + b_shrink,
            inter_on_base.width - ( b_shrink * 2 ),
            inter_on_base.height -( b_shrink * 2 )
        );
        cv::Mat scores(
            inter_on_a.height - inter_on_b_shrinked.height + 1, 
            inter_on_a.width - inter_on_b_shrinked.width + 1, 
            CV_32F
        );
        cv::matchTemplate(
            a(inter_on_a), 
            b(inter_on_b_shrinked), 
            scores, 
            cv::TM_CCORR_NORMED
        );
        double val;
        cv::Point loc;
        cv::minMaxLoc(scores, nullptr, &val, nullptr, &loc);
        cv::Rect cali_inter_on_base(
            inter_on_base.x + loc.x - b_shrink,
            inter_on_base.y + loc.y - b_shrink,
            inter_on_base.width,
            inter_on_base.height
        );
        cv::Rect cali_b_on_base(
            cali_inter_on_base.x - inter_on_b_shrinked.x + b_shrink,
            cali_inter_on_base.y - inter_on_b_shrinked.y + b_shrink,
            b_on_base.width,
            b_on_base.height
        );
        return { cali_inter_on_base, cali_b_on_base };
    }
    static void basic_stitch( 
        const cv::Mat& obj, 
        cv::Mat& base, 
        const cv::Rect& region,
        int i 
    ) {
        // obj.copyTo(base(region));
        cv::Mat overlap;
        // cv::addWeighted(tmp(region), 0.5, obj, 0.5, 0, overlap);
        cv::add(base(region), obj, overlap);
        cv::imwrite("overlap_" + std::to_string(i) + ".tiff", overlap);
        overlap.copyTo(base(region));
    }
    // int row_;
    // int col_;
};

}}