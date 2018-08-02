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
        const std::vector<cv::Point>&             cell_st_pts,
        const std::vector<cv::Point>&             fov_index = {}
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
        for( int i = 0; i < cell_st_pts.size(); i ++ ) {
            auto& m = imgs.at(i);
            auto& st_ps = cell_st_pts.at(i);
            for( auto& mk : m.markers() ) {
                markers_.push_back(
                    cv::Rect(
                        mk.x + st_ps.x,
                        mk.y + st_ps.y,
                        mk.width,
                        mk.height
                    )
                );
            }
        }
        std::cout << "markers, before unique: " << std::endl;
        for( auto& m : markers_ ) {
            std::cout << m << std::endl;
        }
        auto rect_less = [](const cv::Rect& r1, const cv::Rect& r2) {
            if( r1.x == r2.x ) {
                if( r1.y == r2.y ) {
                    if( r1.width == r2.width ) {
                        return r1.height < r2.height;
                    } else return r1.width < r2.width;
                } else return r1.y < r2.y;
            } else return r1.x < r2.x;
        };
        auto rect_eq = []( const cv::Rect& r1, const cv::Rect& r2 ) {
            bool comp[4];
            comp[0] = ( r1.x      == r2.x      );
            comp[1] = ( r1.y      == r2.y      );
            comp[2] = ( r1.width  == r2.width  );
            comp[3] = ( r1.height == r2.height );
            return comp[0] && comp[1] && comp[2] && comp[3];
        };

        std::sort( markers_.begin(), markers_.end(), rect_less);
        auto itr = std::unique( markers_.begin(), markers_.end(), rect_eq );
        markers_.erase(itr, markers_.end());

        std::cout << "markers, after unique: " << std::endl;
        for( auto& m : markers_ ) {
            std::cout << m << std::endl;
        }

        // initial fov index
        if( !fov_index.empty() ) {
            int rs = 0;
            int cs = 0;
            for( auto& fov_p : fov_index ) {
                if( fov_p.x > cs ) cs = fov_p.x;
                if( fov_p.y > rs ) rs = fov_p.y;
            }
            fov_index_ = decltype(fov_index_)(rs, cs);
            int i = 0;
            for( auto& fov_p : fov_index ) {
                fov_index_(fov_p.y, fov_p.x) = i;
                i++;
            }
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

    struct MinCVPixels {
        MinCVPixels(const MultiTiledMat& m)
        : mm_(m)
        {}
        cv::Mat operator()( const CellInfos& cell_infos ) const {
            auto min_cv = std::numeric_limits<FLOAT>::max();
            cv::Mat res;
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res = mm_.cali_imgs_.at(ci.img_idx).mat()(ci).clone();
                }
            }
            return res;
        }
        const MultiTiledMat& mm_;
    };
    struct MinCVAllData {
        struct Result {
            cv::Mat         pixels;
            IdxRect<FLOAT>  cell_info;
        };
        MinCVAllData(const MultiTiledMat& m)
        : mm_(m)
        {}
        Result operator()( const CellInfos& cell_infos ) const {
            auto min_cv = std::numeric_limits<FLOAT>::max();
            Result res;
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res.pixels      = mm_.cali_imgs_.at(ci.img_idx).mat()(ci).clone();
                    res.cell_info   = ci;
                }
            }
            return res;
        }
        const MultiTiledMat& mm_;

    };
    MinCVPixels min_cv_pixels() {
        return MinCVPixels(*this);
    }
    MinCVPixels min_cv_pixels() const {
        return MinCVPixels(*this);
    }
    MinCVAllData min_cv_all_data() {
        return MinCVAllData(*this);
    }
    MinCVAllData min_cv_all_data() const {
        return MinCVAllData(*this);
    }
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
    const std::vector<cv::Rect>& markers() const {
        return markers_;
    }
    std::vector<cv::Rect>& markers() {
        return markers_;
    }
    bool cell_is_marker(const cv::Point& p, cv::Rect& r ) {
        for(auto& m : markers_ ) {
            if( m.contains(p) ) {
                r = m;
                return true;
            }
        }
        return false;
    }
    auto& mats() {
        return cali_imgs_;
    }
    const auto& mats() const {
        return cali_imgs_;
    }
    auto& get_fov_img(int x, int y) {
        return cali_imgs_.at(fov_index_(y, x));
    }
    auto& get_fov_img(int x, int y) const {
        return cali_imgs_.at(fov_index_(y, x));
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
    std::vector<cv::Rect>                      markers_    ;
    cv::Mat_<std::uint16_t>                    fov_index_  ;
};


}