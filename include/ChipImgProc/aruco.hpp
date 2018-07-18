#pragma once
#include <vector>
#include <numeric>
#include <nlohmann/json.hpp>
#include <bitset>
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
};

}