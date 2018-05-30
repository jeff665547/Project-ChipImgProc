#include <ChipImgProc/stitch/position_based.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <iostream>
#include <Nucleona/test/data_dir.hpp>
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
// cv::Mat into_single_rgb( const cv::Mat& mat, int rgb ) {
//     chipimgproc::info(std::cout, mat);
//     cv::Mat res(mat.rows, mat.cols, CV_16UC3);
//     int from_to[] = { rgb, rgb };
//     cv::mixChannels(&mat, 1, &res, 1, from_to, 1);
//     return res;
// }
// cv::Mat red( const cv::Mat& mat ) {
//     return into_single_rgb(mat, 0);
// }
// cv::Mat green( const cv::Mat& mat ) {
//     return into_single_rgb(mat, 1);
// }
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
    imgs.push_back(gray(cv::imread((img_base / "0-0-0.tiff").string())));
    imgs.push_back(gray(cv::imread((img_base / "0-0-1.tiff").string())));
    imgs.push_back(gray(cv::imread((img_base / "0-1-0.tiff").string())));
    imgs.push_back(gray(cv::imread((img_base / "0-1-1.tiff").string())));

    std::vector<cv::Point_<int>> st_ps;
    st_ps.emplace_back(   19 ,    0 );
    st_ps.emplace_back( 2201 ,   19 );
    st_ps.emplace_back(    0 , 2182 );
    st_ps.emplace_back( 2182 , 2201 );
    chipimgproc::stitch::PositionBased pb(2,2);
    auto res = pb(imgs, st_ps);
    chipimgproc::info(std::cout, res);
    cv::imwrite((img_base / "position_based_stitch.tiff").string(), res);
}