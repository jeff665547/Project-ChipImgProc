#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <Nucleona/functional/mutable.hpp>
#include <functional>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/sharpness.h>
#include <Nucleona/app/cli/option_parser.hpp>
namespace chipimgproc{ namespace stitch{ 

struct MarkerBased {
    struct Param {
        cv::Mat image;
        std::vector<cv::Point> feats;
    };
    MarkerBased( int row, int col) 
    : row_(row)
    , col_(col)
    {}

    using FeatureExtractor = std::function<
        std::vector<cv::Point>(const cv::Mat& )
    >;
    void overlap_stitch( 
        cv::Mat& target, 
        std::vector<cv::Rect>& t_rects, 
        const cv::Mat& source,
        const cv::Rect& s_rect 
    ) {
        std::vector<std::pair<cv::Rect, cv::Mat>> rect_list;
        for( auto&& cvr : t_rects ) {
            cv::Rect ovr = s_rect & cvr;
            cv::Mat target_m = target(ovr);
            cv::Mat source_m = source(cv::Rect(
                std::abs(s_rect.x - ovr.x),
                std::abs(s_rect.y - ovr.y),
                ovr.width,ovr.height
            ));

            auto target_sharp = sharpness(target_m);
            auto source_sharp = sharpness(source_m);

            if( source_sharp > target_sharp ) {
                rect_list.push_back( std::pair<cv::Rect, cv::Mat>(
                    ovr, source_m.clone()
                ));
            } else {
                rect_list.push_back( std::pair<cv::Rect, cv::Mat>(
                    ovr, target_m.clone()
                ));
            }
        }
        cv::Mat target_region = target(s_rect);
        source.copyTo(target_region);
        t_rects.push_back(s_rect);
        for( auto&& p : rect_list){
            p.second.copyTo(target(p.first));
        }
    }
    cv::Mat operator()(
        const std::vector<cv::Mat>& imgs,
        const std::vector<
            std::vector<cv::Point>
        >& feats,
        int overlap_marker_row,
        int overlap_marker_col
    ) {
        cv::Mat img;
        Param res { img, {}};
        std::vector<cv::Point> st_ps;
        for( int i = 0; i < row_; i ++) {
            Param row_res;
            std::vector<cv::Point> tmp;
            for ( int j = 0; j < col_; j ++ ) {
                const auto& rm = imgs[i*col_ + j];
                const auto& r_feats = feats[i*col_ + j];
                tmp.push_back(row_stitch(
                    {rm, r_feats},
                    overlap_marker_row,
                    row_res,
                    {0, 0}
                ));
            }
            auto vertical_shift = col_stitch(
                row_res,
                ( col_ * (overlap_marker_col - 1) + 1),
                res,
                {0, 0}
            );
            for ( auto& v : tmp) {
                v += vertical_shift;
                st_ps.push_back(v);
            }
        }

        const auto MAX = std::numeric_limits<int>::max();
        const auto MIN = std::numeric_limits<int>::min();
        cv::Point min(MAX, MAX);
        cv::Point max(MIN, MIN);
        for (int i = 0; i < st_ps.size(); i ++ ) {
            auto& p = st_ps.at(i);
            auto& img = imgs.at(i);
            auto rd = p + cv::Point{ img.cols, img.rows };
            if(p.x < min.x) min.x = p.x;
            if(rd.x > max.x) max.x = rd.x;
            if(p.y < min.y) min.y = p.y;
            if(rd.y > max.y) max.y = rd.y;
        }
        res.image.create(max.y - min.y, max.x - min.x, imgs.at(0).type());

        /**** find ROI region ****/
        std::vector<cv::Rect> rects;
        for( int i = 0; i < row_; i ++ ) {
            for( int j = 0; j < col_; j ++ ) {
                const auto& im  = imgs [i*col_ +j];
                const auto& stp = st_ps[i*col_ +j];
                cv::Rect target_rect(
                   stp.x - min.x, 
                   stp.y - min.y, 
                   im.cols,
                   im.rows
                );
                overlap_stitch(
                    res.image, rects,
                    im, target_rect
                );
            }
        }
        return res.image;
    }
    static bool x_greater( const cv::Point& a, const cv::Point& b) {
        if( a.x == b.x ){
            return a.y > b.y;
        } else {
            return a.x > b.x;
        }
    }
    static bool x_less( const cv::Point& a, const cv::Point& b) {
        if( a.x == b.x ){
            return a.y < b.y;
        } else {
            return a.x < b.x;
        }
    }
    static bool y_greater( const cv::Point& a, const cv::Point& b) {
        if( a.y == b.y ){
            return a.x > b.x;
        } else {
            return a.y > b.y;
        }
    }
    static bool y_less( const cv::Point& a, const cv::Point& b) {
        if( a.y == b.y ){
            return a.x < b.x;
        } else {
            return a.y < b.y;
        }
    }
    static std::vector<cv::Point> x_ordered_great(const std::vector<cv::Point>& points) {
        std::vector<cv::Point> res = points;
        std::sort(res.begin(), res.end(), x_greater);
        return res;
    }
    static std::vector<cv::Point> x_ordered_less(const std::vector<cv::Point>& points) {
        std::vector<cv::Point> res = points;
        std::sort(res.begin(), res.end(), x_less);
        return res;
    }
    static std::vector<cv::Point> y_ordered_great(const std::vector<cv::Point>& points) {
        std::vector<cv::Point> res = points;
        std::sort(res.begin(), res.end(), y_greater);
        return res;
    }
    static std::vector<cv::Point> y_ordered_less(const std::vector<cv::Point>& points) {
        std::vector<cv::Point> res = points;
        std::sort(res.begin(), res.end(), y_less);
        return res;
    }
    static cv::Point row_stitch( const Param& r, int stitch_feat_num, Param& res, const cv::Point& origin) {
        // TODO: rotation analysis
        Param& l = res;
        if(l.feats.size() == 0) {
            // auto target_region = res.image(cv::Rect(origin.x, origin.y, r.image.cols, r.image.rows));
            // r.image.copyTo(target_region);
            res.feats.insert(res.feats.end(), r.feats.begin(),r.feats.end());
            return origin;
        }
        auto middle = (stitch_feat_num - 1) / 2;
        auto l_rm = x_ordered_great(l.feats); // take right most 3 of left image;
        auto r_lm = x_ordered_less (r.feats);

        std::sort(l_rm.begin(), l_rm.begin() + stitch_feat_num, y_less);
        std::sort(r_lm.begin(), r_lm.begin() + stitch_feat_num, y_less);
        auto l_rm_mid = l_rm.at(middle);
        auto r_lm_mid = r_lm.at(middle);

        auto r_start = l_rm_mid - r_lm_mid; // FIXME: not best idea;
        // auto r_target_region = res.image(cv::Rect(origin.x + r_start.x, origin.y + r_start.y, r.image.cols, r.image.rows));
        // r.image.copyTo(r_target_region);

        // FIXME: merge marker, at overlap region, we select right now, but this is not best idea
        res.feats = x_ordered_less(res.feats);
        res.feats.resize(res.feats.size() - stitch_feat_num);
        res.feats.insert(res.feats.end(), r.feats.begin(), r.feats.end());

        int n = res.feats.size();
        return {origin.x + r_start.x, origin.y + r_start.y};
    }
    static cv::Point col_stitch( const Param& d, int stitch_feat_num, Param& res, const cv::Point& origin) {
        // TODO: rotation analysis
        Param& t = res;
        if(t.feats.size() == 0) {
            // auto target_region = res.image(cv::Rect(origin.x, origin.y, d.image.cols, d.image.rows));
            // d.image.copyTo(target_region);
            res.feats.insert(res.feats.end(), d.feats.begin(), d.feats.end());
            return origin;
        }
        auto middle = (stitch_feat_num - 1) / 2;
        auto t_dm = y_ordered_great(t.feats); // take right most 3 of left image;
        auto d_tm = y_ordered_less (d.feats);

        std::sort(t_dm.begin(), t_dm.begin() + stitch_feat_num, x_less);
        std::sort(d_tm.begin(), d_tm.begin() + stitch_feat_num, x_less);
        auto t_dm_mid = t_dm.at(middle);
        auto d_tm_mid = d_tm.at(middle);

        auto d_start = t_dm_mid - d_tm_mid; // FIXME: not best idea;
        // auto d_target_region = res.image(cv::Rect(origin.x + d_start.x, origin.y + d_start.y, d.image.cols, d.image.rows));
        // d.image.copyTo(d_target_region);

        // FIXME: merge marker, at overlap region, we select right now, but this is not best idea
        res.feats = y_ordered_less(res.feats);
        res.feats.resize(res.feats.size() - stitch_feat_num);
        res.feats.insert(res.feats.end(), d.feats.begin(), d.feats.end());

        int n = res.feats.size();
        return {origin.x + d_start.x, origin.y + d_start.y};
    }
private:
    int row_;
    int col_;
};

}}