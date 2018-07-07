#pragma once
#include <ChipImgProc/comb/single_general.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
namespace chipimgproc{ namespace comb{
// using FLOAT = float;
// using GLID = std::uint16_t;
template<
    class FLOAT = float,
    class GLID  = std::uint16_t
>
struct MultiGeneral : public SingleGeneral<FLOAT, GLID> {

    using Base = SingleGeneral<FLOAT, GLID>;
    using CellInfos = std::vector<IdxRect<FLOAT>>;
    auto operator()(
        std::vector<
            boost::filesystem::path
        >                                img_paths,
        const std::vector<cv::Point>&    st_ps
    ) {
        std::vector<typename Base::TiledMatT> tiled_mats  ;
        std::vector<stat::Mats<FLOAT>>        stat_mats_s ;
        int i = 0;
        for( auto&& p : img_paths ) {
            cv::Mat img = cv::imread(p.string(), cv::IMREAD_ANYCOLOR | cv::IMREAD_ANYDEPTH);
            auto [qc, tiled_mat, stat_mats, theta] = Base::operator()(
                img, p.replace_extension("").string()
            );
            tiled_mats.push_back(tiled_mat);
            stat_mats_s.push_back(stat_mats);
            i ++;
        }
        chipimgproc::MultiTiledMat<FLOAT, GLID> multi_tiled_mat(
            tiled_mats, stat_mats_s, st_ps
        );
        return multi_tiled_mat;
    }
};

}}