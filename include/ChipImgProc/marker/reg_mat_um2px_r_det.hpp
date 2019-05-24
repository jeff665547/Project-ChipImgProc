#pragma once
#include <vector>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <Nucleona/tuple.hpp>
#include <range/v3/all.hpp>

namespace chipimgproc::marker {

struct RegMatUm2PxRDet {
    using MKRegion = chipimgproc::marker::detection::MKRegion;
    using DirPointsGroup = std::map<int, std::vector<MKRegion>>;

private:
    void sort_dir(DirPointsGroup& dir_group) const {
    }
    auto x_y_group( const std::vector<MKRegion>& set ) const {
        DirPointsGroup x_group, y_group;
        for(auto&& mk_r : set ) {
            x_group[mk_r.x_i].push_back(mk_r);
            y_group[mk_r.y_i].push_back(mk_r);
        }
        for(auto&& [dir_id, dir] : x_group) {
            std::sort(dir.begin(), dir.end(), [](auto& p0, auto& p1){
                return p0.y < p1.y;
            });
        }
        for(auto&& [dir_id, dir] : y_group) {
            std::sort(dir.begin(), dir.end(), [](auto& p0, auto& p1){
                return p0.x < p1.x;
            });
        }
        return nucleona::make_tuple(
            std::move(x_group), std::move(y_group)
        );
    }
    auto euc_dis(const cv::Point& p0, const cv::Point& p1) const {
        auto diff = p1 - p0;
        return cv::sqrt(diff.x * diff.x + diff.y * diff.y);
    }
    template<class FUNC>
    auto dir_proc(
        std::vector<float>& um2px_rs, 
        const DirPointsGroup& n_group,
        float mk_d_um,
        FUNC&& pred
    ) const {
        for(auto [dir_id, n_dir] : n_group) {
            for(auto p0_p1 : ranges::view::sliding(n_dir, 2) ) {
                auto&& p0 = *p0_p1.begin();
                auto&& p1 = *(p0_p1.begin() + 1);
                if(pred(p0, p1)) continue;
                auto px_dis = euc_dis({p0.x, p0.y}, {p1.x, p1.y});
                um2px_rs.push_back(
                    px_dis / mk_d_um
                );
            }
        }
    }

public:
    float operator()(
        const std::vector<MKRegion>&    mk_regs,
        float                           mk_w_d_um,
        float                           mk_h_d_um,
        std::ostream&                   logger = nucleona::stream::null_out
    ) const {
        std::vector<float> um2px_rs;
        auto [x_group, y_group] = x_y_group(mk_regs);
        dir_proc(um2px_rs, x_group, mk_w_d_um, [](auto&& m0, auto&& m1){ 
            return (m1.y_i - m0.y_i) > 1;
        });
        dir_proc(um2px_rs, y_group, mk_h_d_um, [](auto&& m0, auto&& m1){ 
            return (m1.x_i - m0.x_i) > 1;
        });

        float sum = 0;
        for(auto& f : um2px_rs) {
            logger << "[reg_mat Um2pxR detect]: " << f << '\n';
            sum += f;
        }

        return sum / um2px_rs.size();
    }

};

}