#pragma once
#include <vector>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/warped_mat/basic.hpp>
#include <ChipImgProc/warped_mat/patch.hpp>
namespace chipimgproc::multi_warped_mat {

template<class FOV> 
struct MinCVReducer {
    auto operator()(const std::vector<cv::Mat>& data) const {
        auto mincv_cell = stat::Cell<double>::make(data.at(0));
        std::size_t mincv_i = 0;
        for(std::size_t i = 1; i < data.size(); i ++) {
            auto& px = data.at(i);
            auto cell = stat::Cell<double>::make(px);
            if(cell.cv < mincv_cell.cv) {
                mincv_cell = cell;
                mincv_i = i;
            }
        }
        return warped_mat::Patch(std::move(mincv_cell), data.at(mincv_i));
    }
    auto operator()(const std::vector<warped_mat::Patch>& data) const {
        std::size_t min_i = 0;
        for(std::size_t i = 1; i < data.size(); i ++) {
            if(data.at(i).cv < data.at(min_i).cv) {
                min_i = i;
            }
        }
        return data.at(min_i);
    }
};

template<bool b>
struct MinCVReducer<warped_mat::Basic<b>> {
    auto operator()(const std::vector<std::vector<cv::Mat>>& data) const {
        auto mincv_cell = stat::Cell<double>::make(data.at(0).at(0));
        std::size_t mincv_i = 0;
        for(std::size_t i = 1; i < data.size(); i ++) {
            auto& px = data.at(i).at(0);
            auto cell = stat::Cell<double>::make(px);
            if(cell.cv < mincv_cell.cv) {
                mincv_cell = cell;
                mincv_i = i;
            }
        }
        return warped_mat::Patch(std::move(mincv_cell), data.at(mincv_i).at(0));
    }
};

} // namespace chipimgproc::multi_warped_mat
