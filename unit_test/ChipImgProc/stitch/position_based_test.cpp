#include <ChipImgProc/stitch/position_based.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <iostream>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/stitch/utils.h>
cv::Mat gray( const cv::Mat& mat ) {
    cv::Mat res;
    cv::cvtColor(mat, res, cv::COLOR_RGB2GRAY);
    chipimgproc::info(std::cout, res);
    return res;
}

TEST(position_based_stitch, real_data_test) {
    auto img_base = nucleona::test::data_dir() / "2_20180529113956";
    std::cout << img_base << std::endl;
    std::vector<cv::Mat> imgs;
    imgs.push_back(cv::imread((img_base / "0-0-0.tiff").string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH));
    imgs.push_back(cv::imread((img_base / "0-0-1.tiff").string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH));
    imgs.push_back(cv::imread((img_base / "0-1-0.tiff").string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH));
    imgs.push_back(cv::imread((img_base / "0-1-1.tiff").string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH));

    std::vector<cv::Point_<int>> st_ps;
    st_ps.emplace_back(   19 ,    0 );
    st_ps.emplace_back( 2201 ,   19 );
    st_ps.emplace_back(    0 , 2182 );
    st_ps.emplace_back( 2182 , 2201 );
    chipimgproc::stitch::PositionBased pb;
    auto cali_st_ps = pb(imgs, st_ps, 100);
    auto res = chipimgproc::stitch::add(imgs, cali_st_ps);
    chipimgproc::info(std::cout, res);
    cv::imwrite((img_base / "position_based_stitch.tiff").string(), res);
}