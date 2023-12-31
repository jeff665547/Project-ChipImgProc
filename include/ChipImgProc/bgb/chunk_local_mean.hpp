
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

    auto gen_mk_mask(marker::Layout& layout, const cv::Size& size) const {
        cv::Mat_<std::uint8_t> mask = cv::Mat::ones(size, CV_8UC1);
        for(auto& mk_des : layout.mks) {
            auto& point = mk_des.get_pos(MatUnit::CELL);
            auto& candi_mk = mk_des.get_std_mk(MatUnit::CELL);
            candi_mk.forEach([point, &mask](std::uint8_t& px, const int* pos) {
                int x = point.x + pos[1];
                int y = point.y + pos[0];
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
        , bool                      replace
        , std::ostream&             logger
    ) const
    {
        algo::MatChunk mat_chunk;

        auto& grfimg = grimg;
        grfimg.mat().convertTo( grfimg.mat(), CV_32F );

        // auto grid_chunk_x_size   = grid.cols   / chunk_x_num;
        // auto grid_chunk_y_size   = grid.rows   / chunk_y_num;
        auto grfimg_chunk_x_size = grfimg.mat().cols / chunk_x_num;
        auto grfimg_chunk_y_size = grfimg.mat().rows / chunk_y_num;
        // TODO: remain handling

        auto grid_chunk = mat_chunk(
            grid, 
            chunk_x_num,
            chunk_y_num
        );
        auto grfimg_chunk = mat_chunk(
            grfimg, 
            chunk_x_num,
            chunk_y_num
        );
        auto mk_mask = gen_mk_mask(layout, grid.size());
        std::vector<float> bg_means;
        for( auto&& [g_ch, f_ch] : ranges::view::zip(grid_chunk, grfimg_chunk)) {
            auto&& [gx, gy, g_ch_mat] = g_ch;
            auto&& [_fx, _fy, f_ch_mat] = f_ch;
            auto g_ch_mat_enum = g_ch_mat.rows * g_ch_mat.cols;
            auto trim_num = g_ch_mat_enum * background_trimmed_percent;
            auto trim_num_half = trim_num / 2;

            std::vector<float> means_tmp;
            means_tmp.reserve(g_ch_mat_enum);
            cv::Mat_<std::uint8_t> bin;
            {
                auto tmp = norm_u8(g_ch_mat);
                cv::threshold(tmp, bin, 150, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
            }
            // collect none marker and black cell as sampled backgroud data
            for(int r = 0; r < g_ch_mat.rows; r ++ ) {
                for ( int c = 0; c < g_ch_mat.cols; c ++ ) {
                    auto g_abs_pos_x = gx + c;
                    auto g_abs_pos_y = gy + r;
                    if(mk_mask(g_abs_pos_y, g_abs_pos_x) != 0 && bin(r, c) == 0)
                        means_tmp.push_back(g_ch_mat(r, c));
                
                }
            }
            logger << "means_tmp.size(): " << means_tmp.size() << std::endl;
            // trimmed right and left outlier
            means_tmp |= ranges::action::sort;
            float sum = 0;
            for(std::size_t i = 0; i < means_tmp.size(); i ++ ) {
                sum += means_tmp.at(i);
            }
            // count local background mean
            float chunk_bg_mean = 0;
            if(means_tmp.empty()) { // if no background exist, borrow background from other chunk
                if(bg_means.empty()) {  // if no other chunk, consider current chunk background is 0
                    chunk_bg_mean = 0;
                } else {
                    for(auto&& borrow_bg : bg_means) {
                        chunk_bg_mean += borrow_bg;
                    }
                    chunk_bg_mean /= bg_means.size();
                }
            } else {
                chunk_bg_mean = sum / means_tmp.size();
            }
            logger << "chunk_bg_mean: " << chunk_bg_mean << std::endl;
            // replace raw image pixel, should make a better distribution
            if( replace ) {
                f_ch_mat.mat().template forEach<float>([&chunk_bg_mean](float& px, const int* pos){
                    auto tmp = px - chunk_bg_mean;
                    if( tmp > raw_img_px_floor ) px = tmp;
                    else px = raw_img_px_floor;
                });
            }
            bg_means.push_back(chunk_bg_mean);
        }
        return bg_means;
    }
} chunk_local_mean;

}
