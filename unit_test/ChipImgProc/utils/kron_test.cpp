#include <ChipImgProc/utils/kron.h>
#include <gtest/gtest.h>

TEST(kron_test, basic_test) {
    cv::Mat a = cv::Mat_<std::uint8_t>::eye(2, 2);
    double _b[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    cv::Mat_<double> b(3, 3, _b);
    cv::Mat c = chipimgproc::kron(a, b);
    EXPECT_EQ(c.type(), b.type());
    chipimgproc::typed_mat(c, [&b](auto& mat){
        EXPECT_EQ(mat.rows, 6);
        EXPECT_EQ(mat.cols, 6);
        for(int i = 0; i < mat.rows; i ++) {
            for(int j = 0; j < mat.cols; j ++) {
                if((i < 3 && j < 3) || (i >= 3 && j >= 3)) {
                    EXPECT_DOUBLE_EQ(mat(i, j), b(i % 3, j % 3));
                }
            }
        }
    });
}