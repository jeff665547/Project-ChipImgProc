#pragma once
#include <ChipImgProc/utils.h>
#include <opencv2/opencv.hpp>
namespace chipimgproc::algo {

constexpr struct ScaledMatchTemplate {
    void max(
        cv::Mat tgt, 
        cv::Mat tpl, 
        cv::Mat& score, 
        int method = CV_TM_CCORR_NORMED, 
        int py_down_times = 1,
        cv::Mat mask = cv::Mat()
    ) const {
        cv::Mat stgt;
        cv::Mat stpl;
        cv::Mat smask;

        cv::Mat tmp = tgt.clone();
        for(int i = 0; i < py_down_times; i ++ ) {
            cv::pyrDown(tmp, stgt);
            tmp = stgt.clone();
        }
        tmp = tpl.clone();
        for(int i = 0; i < py_down_times; i ++ ) {
            cv::pyrDown(tmp, stpl);
            tmp = stpl.clone();
        }
        if(!mask.empty()) {
            tmp = mask.clone();
            for(int i = 0; i < py_down_times; i ++ ) {
                cv::pyrDown(tmp, smask);
                tmp = smask.clone();
            }
        }
        cv::Mat_<float> sscore(
            stgt.rows - stpl.rows + 1,
            stgt.cols - stpl.cols + 1
        );
        if(smask.empty()) {
            cv::matchTemplate(stgt, stpl, sscore, method);
        } else {
            cv::matchTemplate(stgt, stpl, sscore, method, smask);
        }
        double min_v, max_v;
        cv::Point min_idx, max_idx;
        cv::minMaxLoc(sscore, &min_v, &max_v, &min_idx, &max_idx);
        
        for(int i = 0; i < py_down_times; i ++) {
            max_idx *= 2;
        }
        auto roi_x = max_idx.x - tpl.cols;
        auto roi_y = max_idx.y - tpl.rows;
        cv::Rect tgt_roi(
            roi_x, roi_y,
            tpl.cols * 2,
            tpl.rows * 2
        );
        cv::Rect score_roi(
            roi_x, roi_y,
            tpl.cols, tpl.rows
        );
        cv::Mat sub_tgt = tgt(tgt_roi);
        score = -1;
        cv::Mat sub_score = score(score_roi);
        if(mask.empty()) {
            cv::matchTemplate(sub_tgt, tpl, sub_score, method);
        } else {
            cv::matchTemplate(sub_tgt, tpl, sub_score, method, mask);
        }
    }
} scaled_match_template;

}