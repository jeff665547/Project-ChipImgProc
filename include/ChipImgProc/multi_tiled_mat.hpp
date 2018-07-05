#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range/irange.hpp>
#include <ChipImgProc/grid_raw_img.hpp>
#include <cstdint>
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/stitch/utils.h>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/stat/mats.hpp>
namespace chipimgproc{

struct IdxRect 
: public cv::Rect
, public stat::Cell 
{
    std::uint16_t img_idx;
};
// using GLID = std::uint16_t;
template<
    class GLID = std::uint16_t
>
struct MultiTiledMat
{
    using IndexType = typename TiledMat<GLID>::IndexType;
    using IndexValue = typename IndexType::value_type;

    MultiTiledMat(
        const std::vector<TiledMat<GLID>>&  imgs,
        const std::vector<stat::Mats>&      stats,
        const std::vector<cv::Point_<int>>& cell_st_pts
    ) {
        // initial cali_imgs_
        for(auto& m: imgs) {
            cali_imgs_.push_back( GridRawImg<GLID>(
                m.get_cali_img(),
                m.glx(),
                m.gly()
            ));
        }

        // initial index_
        std::vector<cv::Mat> indxs;
        for( auto& m : imgs ) {
            indxs.push_back(m.index());
        }
        auto img_rect = stitch::get_full_w_h(
            indxs, cell_st_pts
        );
        index_ = IndexType(img_rect.height, img_rect.width);
        index_.forEach([this](IndexValue& px, const int* pos){
            auto& x = pos[0];
            auto& y = pos[1];
            px = ( y * index_.cols ) + x;
        });
        tiles_.resize(img_rect.height * img_rect.width);
        for( int i = 0; i < imgs.size(); i ++ ) {
            auto& img  = imgs.at(i)       ;
            auto& stat = stats.at(i)      ;
            auto& pt   = cell_st_pts.at(i);
            auto img_index = img.index();
            cv::Rect roi(pt.x, pt.y, img_index.cols, img_index.rows);
            cv::Mat sub_idx = index_(roi);

            // copy index and tiles
            sub_idx.forEach<IndexValue>([this, &img, i, &stat](IndexValue& px, const int* pos){
                auto& x = pos[0];
                auto& y = pos[1];
                auto& src_tile = img.tile_at(y, x);
                IdxRect dst_tile;
                dst_tile.x          = src_tile.x           ;
                dst_tile.y          = src_tile.y           ;
                dst_tile.width      = src_tile.width       ;
                dst_tile.height     = src_tile.height      ;
                dst_tile.img_idx    = i                    ;
                dst_tile.mean       = stat.mean     (y, x) ;
                dst_tile.stddev     = stat.stddev   (y, x) ;
                dst_tile.cv         = stat.cv       (y, x) ;
                dst_tile.num        = stat.num      (y, x) ;
                tiles_.at(px).push_back(dst_tile);
            });
        }
    }
    float mean(std::uint32_t x, std::uint32_t y) {
        auto cell_infos = tiles_.at(index_(y, x));
        float min_cv = std::numeric_limits<float>::max();
        float res;
        for(auto& ci : cell_infos) {
            if(min_cv > ci.cv ) {
                min_cv = ci.cv;
                res = ci.mean;
            }
        }
        return res;
    }
    cv::Mat dump_means() {
        return dump([this](const auto& cell_infos){
            float min_cv = std::numeric_limits<float>::max();
            float res;
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res = ci.mean;
                }
            }
            return res;
        });
    }
    template<class FUNC>
    cv::Mat dump(FUNC&& func) {
        cv::Mat_<float> res(index_.rows, index_.cols);
        res.forEach([this, v_func = FWD(func)](float& value, const int* pos){
            auto& x = pos[0];
            auto& y = pos[1];
            auto cell_infos = tiles_.at(index_(y, x));
            value = v_func(cell_infos);
        });
        return res;
    }
private:
    IndexType                           index_      ;
    std::vector<GridRawImg<GLID>>       cali_imgs_  ;
    std::vector<std::vector<IdxRect>>   tiles_      ;
};


}