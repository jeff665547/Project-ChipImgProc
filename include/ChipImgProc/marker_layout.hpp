#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{
struct MarkerDes {
    cv::Point pos;
    std::vector<cv::Mat> candi_mks;
};
struct MarkerLayout {
    // enum PatternNum {
    //     single, multi
    // };
    enum DistForm {
        uni_mat, random
    };
    cv::Mat_<std::int16_t>  mk_map      ;
    std::vector<MarkerDes>  mks         ;
    std::uint32_t           mk_invl_x_cl;
    std::uint32_t           mk_invl_y_cl;
    // PatternNum  pat_num             ;
    DistForm                dist_form;
};


}