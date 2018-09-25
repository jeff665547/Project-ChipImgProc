
/**
 *  @file       ChipImgProc/bgb/partial_probe_grid_sub_based02.hpp
 *  @author     Chia-Hua Chang
 *  @brief      The background fix algorithm ( number 2 ). 
 *              compute the local background and subtracted. 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/algo/mat_chunk.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/grid_raw_img.hpp>
namespace chipimgproc::bgb {
// TODO document's details
/**
 *  @brief      @copybrief ChipImgProc/bgb/partial_probe_grid_sub_based02.hpp
 */
constexpr struct PartialProbeGridSubBase02
{
    void fill_mask(cv::Mat_<std::uint8_t>& mask, marker::Layout& layout) {
        mask = cv::Mat::ones(mask.size(), CV_8UC1);
        for(auto& mk_des : layout.mks) {
            auto& point = mk_des.get_pos(MatUnit::CELL);
            auto& candi_mk = mk_des.get_candi_mks(MatUnit::CELL).at(0);
            candi_mk.forEach([point, &mask](std::uint8_t& px, int* pos) {
                int x = point.x + pos[0];
                int y = point.y + pos[1];
                if( px == 0 )
                    mask.at(y, x) = 0;
            });
        }
    }
    /**
     *  @brief  Local background process of image.
     *  @param  grid                        The probe grid after ROI ( probe domain image )
     *  @param  image                       The raw image after ROI  ( pixel domain image )
     *  @param  marker_width                The width of marker
     *  @param  marker_height               The height of marker
     *  @param  marker_x_interval           The interval between markers in x direction and is in probe domain
     *  @param  marker_y_interval           The interval between markers in y direction and is in probe domain
     *  @param  chunk_x_num               The x direction number of local segmentation
     *  @param  chunk_y_num               The y direction number of local segmentation
     *  @param  background_trimmed_percent  The percentages of the probes in a single segments, which use to compute the local background
     */
    // template<class GLID>
    using GLID = int;
    auto operator()(
          cv::Mat_<float>& grid // intensities
        , const GridRawImg<GLID>& grimg
        , std::size_t chunk_x_num
        , std::size_t chunk_y_num
        , float background_trimmed_percent
        , marker::Layout& layout
        , std::ostream& logger
    ) const
    {
        algo::MatChunk mat_chunk;

        auto grfimg = grimg.clone();
        grfimg.mat().convertTo( grfimg.mat(), CV_32F );

        auto chunk_x_size = grid.cols / chunk_x_num;
        auto chunk_y_size = grid.rows / chunk_y_num;

        auto grid_chunk = mat_chunk(
            grid, chunk_x_size, chunk_y_size
        );
        auto grfimg_chunk = mat_chunk(
            grfimg, chunk_x_size, chunk_y_size
        );

        for( auto&& [g_ch, f_ch] : ranges::view::zip(grid_chunk, grfimg_chunk)) {
            auto&& [gx, gy, g_ch_mat] = g_ch;
            auto&& [fx, fy, f_ch_mat] = f_ch;
            auto g_ch_mat_enum = g_ch_mat.rows * g_ch_mat.cols;
            auto trim_num = g_ch_mat_enum * background_trimmed_percent;
            auto trim_num_half = trim_num / 2;

            std::vector<float> means_tmp;
            means_tmp.reserve(g_ch_mat_enum);
            for(int r = 0; r < g_ch_mat.rows; r ++ ) {
                for ( int c = 0; c < g_ch_mat.cols; c ++ ) {
                    means_tmp.push_back(g_ch_mat.at(r, c));
                }
            }
            means_tmp = means_tmp | ranges::views::sort;
            for(std::size_t i = trim_num_half; i < ( means_tmp.size() - trim_num + trim_num_half ); i ++ ) {
                // TODO:
            }
        }
    }
} partial_probe_grid_sub_base02;

}}
