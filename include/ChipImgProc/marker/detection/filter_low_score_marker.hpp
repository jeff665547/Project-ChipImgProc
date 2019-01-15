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

        for(auto& id : idx) {
            mk_regs.erase(mk_regs.begin() + id);
        }
    }

} filter_low_score_marker;

}