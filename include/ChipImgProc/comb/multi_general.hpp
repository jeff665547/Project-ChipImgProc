/**
 * @file ChipImgProc/comb/multi_general.hpp
 * @brief The combined chip image process algorthm for multi FOV.
 * @author Chia-Hua Chang
 */
#pragma once
#include <ChipImgProc/comb/single_general.hpp>
#include <ChipImgProc/multi_tiled_mat.hpp>
namespace chipimgproc{ namespace comb{
/**
 *  @brief Chip image process pipeline for multi FOV image.
 *  @tparam FLOAT The float point type used during image process, depend on user application.
 *  @tparam GLID  The integer type used during image prcoess, depend on image size.
 *  @details Multi-FOV image process pipeline runs the Single-FOV algorithm(SingleGeneral) multiple times and stitch. 
 */
template<
    class FLOAT = float,
    class GLID  = std::uint16_t
>
struct MultiGeneral : public SingleGeneral<FLOAT, GLID> {

    using Base = SingleGeneral<FLOAT, GLID>;
    using CellInfos = std::vector<IdxRect<FLOAT>>;
    /**
     *  @brief The main function of multiple image process pipeline.
     *  @details See MultiGeneral and SingleGeneral.
     *  @param img_pats A vector of image paths. Currently only support 16 bit image.
     *  @param st_ps    Grid level image stitch point. This information usually defined in chip FOV spec.
     *  @return The image process result, include grid line, all raw image, pixel statistic data etc. 
     *          See chipimgproc::MultiTiledMat for more detail.
     */
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