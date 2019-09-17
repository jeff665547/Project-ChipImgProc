#include <ChipImgProc/algo/mat_chunk.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST( mat_chunk_test, basic_test ) {
    int row = 6;
    int col = 6;
    cv::Mat_<int> mat(35, 35);
    cv::randu(mat, cv::Scalar(0), cv::Scalar(100));

    chipimgproc::algo::MatChunk mat_chunk;
    auto chunks = mat_chunk((cv::Mat&)mat, row, col);
    EXPECT_EQ((int)ranges::distance(chunks), (row * col));
    auto itr = chunks.begin();
    int exp_x = 0;
    int exp_y = 0;
    for(int i = 0; i < row; i ++) {
        int mat_row = -1;
        int mat_y = -1;
        for(int j = 0; j < col; j ++) {
            auto&& [x, y, ch_mat] = *itr;

            if(mat_row >= 0) { 
                EXPECT_EQ(mat_row, ch_mat.rows);
            }
            mat_row = ch_mat.rows;

            if(mat_y >= 0 ) { 
                EXPECT_EQ(mat_y, y);
            }
            mat_y = y;
            
            EXPECT_EQ(x, exp_x);
            exp_x = x + ch_mat.cols;
            itr++;
        }
        EXPECT_EQ(exp_x, 35);
        exp_x = 0;
        EXPECT_EQ(mat_y, exp_y);
        exp_y = mat_y + mat_row;
    }
    EXPECT_EQ(exp_y, 35);
}