#pragma once
#include <vector>
#include <numeric>
#include <nlohmann/json.hpp>
#include <bitset>
#include <ChipImgProc/utils.h>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc{ 

struct ArUco {
    static int bin_to_dec( const std::vector<bool>& bin_vec ) {
        return std::accumulate(bin_vec.rbegin(), bin_vec.rend(), 0, [](std::int64_t x, std::int64_t y) {
            return (x << 1) + y;
        });
    }
    static std::int64_t bin_to_dec( const nlohmann::json& bin_json_arr ) {
        std::int64_t n = 0;
        return std::accumulate(bin_json_arr.begin(), bin_json_arr.end(), n, [](const auto& a, const auto& b){
            std::int64_t ia = a;
            std::int64_t ib = b;
            std::int64_t tmp = ia << 1ULL;
            tmp += ib;
            return tmp;
        });
    }
    static std::int64_t mat_to_dec( 
        const cv::Mat& m, 
        std::ostream& logger = nucleona::stream::null_out
    ) {
        cv::Mat_<std::uint8_t> tmp = m;
        std::int64_t n = 0;
        return std::accumulate(tmp.begin(), tmp.end(), n, [](const auto& a, const auto& b){
            std::int64_t ia = a;
            std::int64_t ib = b;
            std::int64_t tmp = ia << 1ULL;
            tmp += ib;
            return tmp;
        });
    }
    static cv::Mat dec_to_mat(std::int64_t n, int row, int col) {
        static const std::int64_t one = 1;
        cv::Mat_<std::uint8_t> res(row, col);
        for( int i = row -1; i >= 0; i --) {
            for( int j = col - 1; j >=0 ; j --) {
                res(i, j) = (n & one); 
                n = n >> 1;
            }
        }
        return res;
    } 
};

}