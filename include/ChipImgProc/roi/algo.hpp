#pragma once
#include <ChipImgProc/marker_layout.hpp>
#include <ChipImgProc/roi/result.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/tiled_mat.hpp>
namespace chipimgproc{ namespace roi{

struct Algo {
    using TID = int;
    using GLID = int;
    void process_matrix_layout(
        const MarkerLayout&     mk_layout,
        stat::Mats&             raw_smats,
        TiledMat<TID, GLID>&    tiled_src
    ) {
        auto tgt = binarize(raw_smats.mean); // TODO:
        auto sp_h = tgt.rows / mk_layout.mk_map.rows;
        auto sp_w = tgt.cols / mk_layout.mk_map.cols;
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
                cv::Mat sub_tgt = tgt(match_region); // not correct
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
            }
        }
        
    }
    void operator()(const MarkerLayout& ml) {
        if(ml.dist_form == MarkerLayout::uni_mat) {
            // split image
        }
    }
};


}}