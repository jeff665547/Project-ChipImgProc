#pragma once
#include <unordered_map>
#include <ChipImgProc/utils.h>
#include <nlohmann/json.hpp>
namespace chipimgproc::aruco {

struct MarkerMap {
    MarkerMap(const nlohmann::json& id_map) 
    : idx2sub_()
    , sub2idx_() 
    {
        idx2sub_.clear();
        int rows = 0;
        int cols = 0;
        for(auto& [key, value]: id_map.items()) {
            auto idx = std::stoi(key);
            auto pos = cv::Point(value[0], value[1]);
            idx2sub_[idx] = pos;
            rows = std::max(rows, pos.y);
            cols = std::max(cols, pos.x);
        }
        sub2idx_.create(rows + 1, cols + 1);
        sub2idx_.setTo(-1);
        for(auto& [key, value]: id_map.items()) {
            auto idx = std::stoi(key);
            sub2idx_(value[1], value[0]) = idx;
        }
    }

    std::vector<std::int32_t> get_marker_indices(void) const {
        std::vector<std::int32_t> res;
        res.reserve(idx2sub_.size());
        for(auto&& [k, v]: idx2sub_) {
            res.emplace_back(k);
        }
        return res;
    }

    std::int32_t get_idx(cv::Point pos) const {
        if( pos.x < 0 || pos.x >= sub2idx_.cols || 
            pos.y < 0 ||pos.y >= sub2idx_.rows
        ) throw std::invalid_argument("position not found");
        return sub2idx_(pos.y, pos.x);
    }

    cv::Point get_sub(std::int32_t idx) const {
        try {
            return idx2sub_.at(idx);
        } catch(...) {
            throw std::invalid_argument("index not found");
        }
    }

private:
    std::unordered_map<std::int32_t, cv::Point> idx2sub_;
    cv::Mat_<std::int32_t>                      sub2idx_;
};
    
}