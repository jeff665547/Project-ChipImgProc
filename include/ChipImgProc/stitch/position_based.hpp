#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{

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
        cv::Mat res(w_h.y, w_h.x, imgs.at(0).type());
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

}