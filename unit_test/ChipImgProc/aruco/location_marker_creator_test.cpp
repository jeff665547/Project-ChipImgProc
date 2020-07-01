#include <ChipImgProc/aruco/location_mark_creator.hpp>
#include <gtest/gtest.h>
#include <ChipImgProc/utils.h>

TEST(location_marker_creator, basic_test) {
    auto [templ0, mask0] = chipimgproc::aruco::create_location_marker(
        50, 40, 3, 5, 2.414
    );
    auto [templ1, mask1] = chipimgproc::aruco::create_location_marker(
        50 * 2.414, 
        40 * 2.414, 
        3  * 2.414, 
        5  * 2.414 
    );
    ASSERT_EQ(templ0.rows, templ1.rows);
    ASSERT_EQ(templ0.cols, templ1.cols);
    ASSERT_EQ(mask0.rows, mask1.rows);
    ASSERT_EQ(mask0.cols, mask1.cols);
    std::cout << "templ (row, col): " << '(' 
        << templ0.rows << ',' << templ0.cols 
        << ")\n"
    ;
    std::cout << "mask (row, col): " << '(' 
        << mask0.rows << ',' << mask0.cols 
        << ")\n"
    ;
    auto score = chipimgproc::match_template(templ0, templ1);
    std::cout << score(0,0) << std::endl;
    // for(int i = 0; i < templ0.rows; i ++) {
    //     for(int j = 0; j < templ0.cols; j ++) {
    //         std::cout << '(' << i << ',' << j << ")\n";
    //         ASSERT_EQ(templ0(i, j), templ1(i, j));
    //     }
    // }
    // for(int i = 0; i < mask0.rows; i ++) {
    //     for(int j = 0; j < mask0.cols; j ++) {
    //         std::cout << '(' << i << ',' << j << ")\n";
    //         ASSERT_EQ(mask0(i, j), mask1(i, j));
    //     }
    // }
}