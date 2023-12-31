#include <ChipImgProc/utils.h>
#include <ChipImgProc/logger.hpp>
#include <iostream>
#include <vector>
namespace chipimgproc{ namespace stitch{

cv::Rect get_full_w_h( 
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
    return cv::Rect(
        min.x, 
        min.y,
        max.x - min.x,
        max.y - min.y
    );
}
std::vector<cv::Point_<int>> normalize_points( 
    const cv::Point_<int>& minp,
    const std::vector<cv::Point_<int>>& ps
)
{
    std::vector<cv::Point_<int>> res;
    for( auto&& p : ps ) {
        auto tmp = p - minp;
        if( tmp.x < 0 || tmp.y < 0 ) {
            throw std::runtime_error("normalize_points assert fail");
        }
        res.push_back(tmp);
    }
    return res;
}
cv::Mat add(
    const std::vector<cv::Mat>& imgs, 
    const std::vector<cv::Point_<int>>& st_ps
){
    auto res_roi = get_full_w_h(imgs, st_ps);
    cv::Mat res(res_roi.height, res_roi.width, imgs.front().type());
    auto norm_st_ps = normalize_points(
        cv::Point_<int>(res_roi.x, res_roi.y),
        st_ps
    );
    std::vector<cv::Rect> added_rois;
    for( int i = 0; i < imgs.size(); i ++ ) {
        auto& img_i = imgs.at(i);
        auto norm_ps(norm_st_ps.at(i));
        cv::Rect roi(
            norm_ps.x, norm_ps.y,
            img_i.cols, img_i.rows
        );
        img_i.copyTo(res(roi));
        if( !added_rois.empty() ) {
            for( int j = 0; j < added_rois.size(); j ++) {
                auto& aroi = added_rois.at(j);
                auto& img_j = imgs.at(j);
                cv::Rect inter(roi & aroi);
                cv::Mat overlap;
                chipimgproc::log.trace("roi: [{},{},{},{}]", roi.x, roi.y, roi.width, roi.height);
                chipimgproc::log.trace("aroi: [{},{},{},{}]", aroi.x, aroi.y, aroi.width, aroi.height);
                chipimgproc::log.trace("inter: [{},{},{},{}]", inter.x, inter.y, inter.width, inter.height);
                if(inter.area() > 0 ) {
                    cv::addWeighted(
                        img_i(cv::Rect(
                            inter.x - roi.x,
                            inter.y - roi.y,
                            inter.width,
                            inter.height
                        )), 
                        0.5,
                        img_j(cv::Rect(
                            inter.x - aroi.x,
                            inter.y - aroi.y,
                            inter.width,
                            inter.height
                        )),
                        0.5,
                        0,
                        overlap
                    );
                    overlap.copyTo(res(inter));
                }
            }
        }
        added_rois.push_back(roi);
    }
    return res;
}
}}