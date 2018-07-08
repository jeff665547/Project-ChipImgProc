#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range.hpp>
#include <ChipImgProc/grid_raw_img.hpp>
#include <cstdint>
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/stitch/utils.h>
#include <ChipImgProc/stat/cell.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/wrapper/indexed_range.hpp>
namespace chipimgproc{

template<class FLOAT = float>
struct IdxRect 
: public cv::Rect
, public stat::Cell<FLOAT> 
{
    std::uint16_t img_idx;
};
namespace detail{

template<class FLOAT = float>
using CellInfos_    = std::vector<IdxRect<FLOAT>>;

template<class FLOAT = float>
using Tiles_        = std::vector<CellInfos_<FLOAT>>;

template<class GLID = std::uint16_t>
using IndexType_    = typename TiledMat<GLID>::IndexType;

template<
    class FLOAT = float,
    class GLID  = std::uint16_t
>
using IndexedRange = wrapper::IndexedRange<
    detail::Tiles_<FLOAT>,
    detail::IndexType_<GLID>
>;

template<class GLID = std::uint16_t>
struct IndexWrapper {
    using IndexType = IndexType_<GLID>;
    IndexType index_;
};
template<class FLOAT = float>
struct TilesWrapper {
    using Tiles = Tiles_<FLOAT>;
    Tiles tiles_;
};


}
// using GLID = std::uint16_t;
template<
    class FLOAT = float,
    class GLID  = std::uint16_t
>
struct MultiTiledMat 
: protected detail::TilesWrapper<FLOAT>
, protected detail::IndexWrapper<GLID>
, public detail::IndexedRange<FLOAT, GLID>
{
    using IndexType  = typename detail::IndexWrapper<GLID>::IndexType;
    using IndexValue = typename IndexType::value_type;
    using CellInfos  = detail::CellInfos_<FLOAT>;
    using Tiles      = typename detail::TilesWrapper<FLOAT>::Tiles;
    using This       = MultiTiledMat<FLOAT, GLID>;

    MultiTiledMat(
        const std::vector<TiledMat<GLID>>&        imgs,
        const std::vector<stat::Mats<FLOAT>>&     stats,
        const std::vector<cv::Point>&             cell_st_pts
    )
    : detail::IndexedRange<FLOAT, GLID>(
        detail::TilesWrapper<FLOAT>::tiles_,
        detail::IndexWrapper<GLID>::index_
    )
    {
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
        this->index_ = IndexType(img_rect.height, img_rect.width);
        this->index_.forEach([this](IndexValue& px, const int* pos){
            auto& r = pos[0];
            auto& c = pos[1];
            px = ( r * this->index_.cols ) + c;
        });
        this->tiles_.resize(img_rect.height * img_rect.width);
        for( int i = 0; i < imgs.size(); i ++ ) {
            auto& img  = imgs.at(i)       ;
            auto& stat = stats.at(i)      ;
            auto& pt   = cell_st_pts.at(i);
            auto img_index = img.index();
            cv::Rect roi(pt.x, pt.y, img_index.cols, img_index.rows);
            cv::Mat sub_idx = this->index_(roi);

            // copy index and tiles
            sub_idx.forEach<IndexValue>([this, &img, i, &stat](IndexValue& px, const int* pos){
                auto& r = pos[0];
                auto& c = pos[1];
                auto& src_tile = img.tile_at(r, c);
                IdxRect dst_tile;
                dst_tile.x          = src_tile.x           ;
                dst_tile.y          = src_tile.y           ;
                dst_tile.width      = src_tile.width       ;
                dst_tile.height     = src_tile.height      ;
                dst_tile.img_idx    = i                    ;
                dst_tile.mean       = stat.mean     (r, c) ;
                dst_tile.stddev     = stat.stddev   (r, c) ;
                dst_tile.cv         = stat.cv       (r, c) ;
                dst_tile.num        = stat.num      (r, c) ;
                this->tiles_.at(px).push_back(dst_tile);
            });
        }
    }
    static constexpr struct MinCVMean {
        FLOAT operator()( const CellInfos& cell_infos ) const {
            auto min_cv = std::numeric_limits<FLOAT>::max();
            FLOAT res;
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res = ci.mean;
                }
            }
            return res;
        }
    } min_cv_mean{};

    auto rows() const {
        return this->index_.rows;
    }
    auto cols() const {
        return this->index_.cols;
    }
    template<class CELL_INFOS_FUNC = decltype(min_cv_mean)&>
    decltype(auto) at(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean
    ) const {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class CELL_INFOS_FUNC = decltype(min_cv_mean)&>
    decltype(auto) at(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean
    ) {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class CELL_INFOS_FUNC = decltype(min_cv_mean)&>
    decltype(auto) operator()(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean
    ) const {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class CELL_INFOS_FUNC = decltype(min_cv_mean)&>
    decltype(auto) operator()(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean
    ) {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class FUNC = decltype(min_cv_mean)&>
    cv::Mat dump(FUNC&& func = min_cv_mean) {
        cv::Mat_<FLOAT> res(this->index_.rows, this->index_.cols);
        res.forEach([this, v_func = FWD(func)](FLOAT& value, const int* pos){
            auto& r = pos[0];
            auto& c = pos[1];
            auto cell_infos = this->tiles_.at(this->index_(r, c));
            value = v_func(cell_infos);
        });
        return res;
    }

private:
    template<
        class THIS__, 
        class CELL_INFOS_FUNC = decltype(min_cv_mean)
    >
    static decltype(auto) at_impl(
        THIS__&             this_, 
        std::uint32_t       row, 
        std::uint32_t       col, 
        CELL_INFOS_FUNC&&   cell_infos_func = min_cv_mean
    ) {
        auto&& cell_infos = this_.tiles_.at(
            this_.index_(row, col)
        );
        return cell_infos_func(FWD(cell_infos));
    }
    std::vector<GridRawImg<GLID>>              cali_imgs_  ;
};


}