#include <ChipImgProc/algo/mat_chunk.hpp>
#include <Nucleona/app/cli/gtest.hpp>

TEST( mat_chunk_test, basic_test ) {
    cv::Mat_<int> mat(35, 35);
    cv::randu(mat, cv::Scalar(0), cv::Scalar(100));

    chipimgproc::algo::MatChunk mat_chunk;
    auto chunks = mat_chunk((cv::Mat&)mat, 6, 6);
    for(auto&& [x, y, ch_mat] : chunks) {
        std::cout << x << ',' << y << std::endl;
        std::cout << ch_mat.cols << ',' << ch_mat.rows << std::endl;
        std::cout << std::endl;
    }
}