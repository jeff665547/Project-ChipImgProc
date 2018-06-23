#pragma once
#include <ChipImgProc/marker_layout.hpp>
#include <ChipImgProc/roi/result.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/tiled_mat.hpp>
#include <map>
namespace chipimgproc{ namespace roi{

struct UniMarkerMatLayout {
    using GLID = int;
    cv::Mat binarize(const cv::Mat_<float>& m) {
        auto trimmed_mean = trim_outlier(m.clone(), 0, 0.02); // TODO: smarter way
        cv::Mat_<std::uint8_t> bin;
        cv::normalize(trimmed_mean, bin, 0, 255, cv::NORM_MINMAX, bin.depth());
        return bin;
    }
    auto check_marker_position(
        const cv::Mat_<std::uint16_t>& tgt_mk_pos_x,
        const cv::Mat_<std::uint16_t>& tgt_mk_pos_y,
        const MarkerLayout& mk_layout,
        int threshold = 2
    ) {
        cv::Point op(tgt_mk_pos_x(0, 0), tgt_mk_pos_y(0, 0));
        std::map<cv::Point, int> vote_tab;
        for( int r = 0; r < tgt_mk_pos_x.rows; r ++ ) {
            for ( int c = 0; c < tgt_mk_pos_x.cols; c ++ ) {
                std::uint16_t x = tgt_mk_pos_x(r, c);
                std::uint16_t y = tgt_mk_pos_y(r, c);
                cv::Point p(x, y);
                p.x -= (mk_layout.mk_invl_x_cl * c);
                p.y -= (mk_layout.mk_invl_y_cl * r);
                vote_tab[p] ++;
            }
        }
        auto max_vote = vote_tab.begin();
        for ( auto itr = vote_tab.begin(); itr != vote_tab.end(); itr ++ ) {
            if( itr->second > max_vote->second ) {
                max_vote = itr;
            }
        }
        struct {
            bool qc_pass;
            cv::Point org_point;
        } res {
            max_vote->second > threshold,
            max_vote->first
        };
        return res;
    }
    bool operator()( 
        const MarkerLayout&     mk_layout,
        stat::Mats&             raw_smats,
        TiledMat<GLID>&         tiled_src
    ) {
        auto tgt = binarize(raw_smats.mean); // TODO:
        auto sp_h = tgt.rows / mk_layout.mk_map.rows;
        auto sp_w = tgt.cols / mk_layout.mk_map.cols;
        cv::Mat_<std::uint16_t> found_mk_pos_x(
            mk_layout.mk_map.rows,
            mk_layout.mk_map.cols
        );
        cv::Mat_<std::uint16_t> found_mk_pos_y(
            mk_layout.mk_map.rows,
            mk_layout.mk_map.cols
        );
        for( int r = 0; r < mk_layout.mk_map.rows; r ++ ) {
            for( int c = 0; c < mk_layout.mk_map.cols; c ++ ) {
                std::int16_t mk_des_idx = mk_layout.mk_map(c, r);
                auto& mk_des = mk_layout.mks.at(mk_des_idx);
                auto& candi_mks = mk_des.candi_mks;
                cv::Rect match_region(
                    c * sp_w,
                    r * sp_h,
                    sp_w,
                    sp_h
                );
                cv::Mat sub_tgt = tgt(match_region); // not correct?
                cv::Mat_<float> sub_score;
                for(auto& mk : candi_mks) {
                    cv::Mat_<float> sub_candi_score(
                        sub_tgt.cols - mk.cols + 1,
                        sub_tgt.rows - mk.rows + 1
                    );
                    cv::matchTemplate(sub_tgt, mk, sub_candi_score, CV_TM_CCORR_NORMED);
                    if(sub_score.empty()) 
                        sub_score = sub_candi_score;
                    else 
                        sub_score += sub_candi_score;
                }
                double min, max;
                cv::Point min_loc, max_loc;
                cv::minMaxLoc(sub_score, &min, &max, &min_loc, &max_loc);
                max_loc.x += match_region.x;
                max_loc.y += match_region.y;
                found_mk_pos_x(r, c) = max_loc.x;
                found_mk_pos_y(r, c) = max_loc.y;
            }
        }
        auto check_res = check_marker_position(
            found_mk_pos_x, 
            found_mk_pos_y,
            mk_layout
        );
        cv::Rect r(
            check_res.org_point.x,
            check_res.org_point.y,
            check_res.org_point.x + 
                mk_layout.mk_invl_x_cl * mk_layout.mk_map.cols,
            check_res.org_point.y + 
                mk_layout.mk_invl_y_cl * mk_layout.mk_map.rows
        );
        tiled_src.roi(r);
        raw_smats.roi(r);
        
        return check_res.qc_pass;
        
    }
};


}}