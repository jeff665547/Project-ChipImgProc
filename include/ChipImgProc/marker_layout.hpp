#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{
struct MarkerDes {
    cv::Point pos;
    std::vector<cv::Mat> candi_mks;
};
struct MarkerLayout {
    enum PatternNum {
        single, multi
    };
    enum DistForm {
        uni_mat, random
    };
    const MarkerDes& get_marker_des(int r, int c) const {
        if( dist_form != uni_mat) {
            throw std::runtime_error("get_marker_des(r,c) only support uni_mat form");
        }
        return mks.at(mk_map(r, c));
    }
    cv::Mat_<std::int16_t>  mk_map      ;
    std::vector<MarkerDes>  mks         ;
    std::uint32_t           mk_invl_x_cl;
    std::uint32_t           mk_invl_y_cl;
    PatternNum              pat_num     ;
    DistForm                dist_form   ;
};


}