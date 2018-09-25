
/**
 *  @file       ChipImgProc/bgb/chunk_local_mean.hpp
 *  @author     Chia-Hua Chang
 *  @brief      The background fix algorithm ( number 2 ). 
 *              compute the local background and subtracted. 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/algo/mat_chunk.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/grid_raw_img.hpp>
#include <range/v3/action/sort.hpp>

namespace chipimgproc::bgb {
// TODO document's details
/**
 *  @brief      @copybrief ChipImgProc/bgb/chunk_local_mean.hpp
 */
constexpr struct ChunkLocalMean
{
    constexpr static float raw_img_px_floor = 1e-10;

    auto gen_mk_mask(marker::Layout& layout) const {
        cv::Mat_<std::uint8_t> mask = cv::Mat::ones(mask.size(), CV_8UC1);
        for(auto& mk_des : layout.mks) {
            auto& point = mk_des.get_pos(MatUnit::CELL);
            auto& candi_mk = mk_des.get_candi_mks(MatUnit::CELL).at(0);
            candi_mk.forEach([point, &mask](std::uint8_t& px, const int* pos) {
                int x = point.x + pos[0];
                int y = point.y + pos[1];
                mask(y, x) = 0;
            });
        }
        return mask;
    }
    /**
     *  @brief  Local background process of image.
     *  @param  grid                        The probe grid after ROI ( probe domain image )
     *  @param  image                       The raw image after ROI  ( pixel domain image )
     *  @param  chunk_x_num               The x direction number of local segmentation
     *  @param  chunk_y_num               The y direction number of local segmentation
     *  @param  background_trimmed_percent  The percentages of the probes in a single segments, which use to compute the local background
     */
    // using GLID = int;
    template<class GLID>
    auto operator()(
          cv::Mat_<float>&          grid    // intensities
        , GridRawImg<GLID>&         grimg   // NOTE: convert to float image
        , std::size_t               chunk_x_num
        , std::size_t               chunk_y_num
        , float                     background_trimmed_percent
        , marker::Layout&           layout
        , std::ostream&             logger
    ) const
    {
        algo::MatChunk mat_chunk;

        auto& grfimg = grimg;
        grfimg.mat().convertTo( grfimg.mat(), CV_32F );

        auto chunk_x_size = grid.cols / chunk_x_num;
        auto chunk_y_size = grid.rows / chunk_y_num;

        auto grid_chunk = mat_chunk(
            grid, chunk_x_size, chunk_y_size
        );
        auto grfimg_chunk = mat_chunk(
            grfimg, chunk_x_size, chunk_y_size
        );
        auto mk_mask = gen_mk_mask(layout);
        std::vector<float> bg_means;
        for( auto&& [g_ch, f_ch] : ranges::view::zip(grid_chunk, grfimg_chunk)) {
            auto&& [gx, gy, g_ch_mat] = g_ch;
            auto&& [_fx, _fy, f_ch_mat] = f_ch;
            auto g_ch_mat_enum = g_ch_mat.rows * g_ch_mat.cols;
            auto trim_num = g_ch_mat_enum * background_trimmed_percent;
            auto trim_num_half = trim_num / 2;

            std::vector<float> means_tmp;
            means_tmp.reserve(g_ch_mat_enum);
            // collect none marker masked means
            for(int r = 0; r < g_ch_mat.rows; r ++ ) {
                for ( int c = 0; c < g_ch_mat.cols; c ++ ) {
                    auto g_abs_pos_x = gx + c;
                    auto g_abs_pos_y = gy + r;
                    if(mk_mask(g_abs_pos_y, g_abs_pos_x) != 0)
                        means_tmp.push_back(g_ch_mat(r, c));
                }
            }
            // trimmed right and left outlier
            means_tmp |= ranges::action::sort;
            float sum = 0;
            for(std::size_t i = trim_num_half; i < ( means_tmp.size() - trim_num + trim_num_half ); i ++ ) {
                sum += means_tmp.at(i);
            }
            // count local background mean
            const auto chunk_bg_mean = sum / means_tmp.size();
            // replace raw image pixel, should make a better distribution
            f_ch_mat.mat().template forEach<float>([&chunk_bg_mean](float& px, const int* pos){
                auto tmp = px - chunk_bg_mean;
                if( tmp > raw_img_px_floor ) px = tmp;
                else px = raw_img_px_floor;
            });
            bg_means.push_back(chunk_bg_mean);
        }
        return bg_means;
    }
} chunk_local_mean;

}
