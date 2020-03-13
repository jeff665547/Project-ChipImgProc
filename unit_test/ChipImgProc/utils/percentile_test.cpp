#include <ChipImgProc/utils/percentile.hpp>
#include <Nucleona/app/cli/gtest.hpp>
#include <iostream>
TEST(utils, percentile) {
    std::vector<int> x({1,2,3,4,5,6});
    EXPECT_DOUBLE_EQ(chipimgproc::utils::percentile(x, 50), 3.5);
    EXPECT_DOUBLE_EQ(chipimgproc::utils::percentile(x, 0), 1);
    EXPECT_DOUBLE_EQ(chipimgproc::utils::percentile(x, 5), 1.25);
}