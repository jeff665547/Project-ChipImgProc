#include <ChipImgProc/stitch/position_based.hpp>
#include <gtest/gtest.h>
#include <iostream>
TEST(position_based_stitch, literal_data_test){
    chipimgproc::stitch::PositionBased pb(2, 2);
    std::vector<cv::Mat> imgs;
    std::vector<cv::Point_<int>> st_ps;

    cv::Mat_<int> lt(3,3);
    lt << 0, 1, 1
       ,  0, 1, 1
       ,  1, 1, 1
    ;
    imgs.push_back(lt);
    st_ps.push_back(cv::Point_<int>(0, 0));

    cv::Mat_<int> rt(3,3);
    rt << 1, 1, 0
       ,  1, 1, 0
       ,  1, 1, 1
    ;
    imgs.push_back(rt);
    st_ps.push_back(cv::Point_<int>(1, 0));

    cv::Mat_<int> lb(3,3);
    lb << 1, 1, 1
       ,  0, 1, 1
       ,  0, 1, 1
    ;
    imgs.push_back(lb);
    st_ps.push_back(cv::Point_<int>(0, 2));

    cv::Mat_<int> rb(3,3);
    rb << 1, 1, 1
       ,  1, 1, 0
       ,  1, 1, 0
    ;
    imgs.push_back(rb);
    st_ps.push_back(cv::Point_<int>(1, 2));

    auto res = pb(imgs, st_ps);
    cv::Mat_<int> expect(5, 4);
    expect << 
        0, 1, 1, 0,
        0, 1, 1, 0,
        1, 1, 1, 1,
        0, 1, 1, 0,
        0, 1, 1, 0
    ;
    cv::Mat comp_res;
    cv::compare(res, expect, comp_res, cv::CMP_EQ); // true: 255, false: 0
    
    comp_res.forEach<std::uint8_t>([](std::uint8_t c, const int* position){
        EXPECT_EQ(c, 255);
    });
}