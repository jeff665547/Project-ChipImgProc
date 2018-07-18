#pragma once
#include <vector>
#include <numeric>
#include <nlohmann/json.hpp>
namespace chipimgproc{ 

struct ArUco {
    static int bin_to_dec( const std::vector<bool>& bin_vec ) {
        return std::accumulate(bin_vec.rbegin(), bin_vec.rend(), 0, [](int x, int y) {
            return (x << 1) + y;
        });
    }
    static int bin_to_dec( const nlohmann::json& bin_json_arr ) {
        return std::accumulate(bin_json_arr.rbegin(), bin_json_arr.rend(), 0, [](const auto& a, const auto& b){
            int ia = a;
            int ib = b;
            return (ia << 1) + ib;
        });
    }
};

}