#pragma once
#include <vector>
#include <ChipImgProc/marker/detection/mk_region.hpp>
namespace chipimgproc::marker::detection {

constexpr struct FilterLowScoreMarker {

    auto operator()(std::vector<MKRegion>& mk_regs) const {
        // TODO: fix use of erase
        std::vector<int> idx;
        for(int i = 0; i < mk_regs.size(); i ++ ) {
            idx.push_back(i);
        }
        std::sort(idx.begin(), idx.end(), [&mk_regs](auto a, auto b){
            return mk_regs.at(a).score < mk_regs.at(b).score;
        });
        auto trim_num = mk_regs.size() / 4;
        idx.resize(trim_num);
        std::sort(idx.begin(), idx.end(), std::greater<int>());
        std::vector<cv::Point> low_score_mks_idx;
        for(auto& id : idx) {
            auto& mk_r = mk_regs.at(id);
            low_score_mks_idx.push_back(
                cv::Point(mk_r.x_i, mk_r.y_i)
            );
            mk_regs.erase(mk_regs.begin() + id);
        }
        return low_score_mks_idx;
    }

} filter_low_score_marker;

}