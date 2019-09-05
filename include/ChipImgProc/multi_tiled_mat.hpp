/**
 * @file multi_tiled_mat.hpp
 * @author Chia-Hua Chang(johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::MultiTiledMat
 * 
 */
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
#include <mutex>
#include <memory>
#include <ChipImgProc/logger.hpp>
namespace chipimgproc{
/**
 * @brief Grid cell, include location, width, height, 
 *        statistic data, and FOV image id.
 * @details The FOV image ID is a sequencial number 
 *   to identify the current cell belong to which FOV, 
 *   and this ID usually numbering by the FOV row major order.
 * 
 * @tparam FLOAT The float point type used in this data structure, 
 *   and is use to trade off the performance and numerical accuracy.
 */
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
/**
 * @brief Container of multiple tiled fov images. 
 *   Provide chip level coordinates access to cell data, 
 *   And lazy evaluate the cell statistic values.
 * 
 * @tparam FLOAT The float point type used in this data structure, 
 *   and is use to trade off the performance and numerical accuracy.
 * @tparam GLID The grid line type, 
 *   and is use to limit the memory usage of the grid line storages.
 */
template<
    class FLOAT = float,
    class GLID  = std::uint16_t
>
struct MultiTiledMat 
: protected detail::TilesWrapper<FLOAT>     // tile storage, workaround for member initialize order 
, protected detail::IndexWrapper<GLID>      // index storage, workaround for member initialize order 
, public detail::IndexedRange<FLOAT, GLID>  // provide range based for iterator
{
    /**
     * @brief The float point type use in current type.
     * 
     */
    using FloatType = FLOAT;
    /**
     * @brief The (row, col) coordinate to one dimension integer transform matrix type.
     * 
     */
    using IndexType  = typename detail::IndexWrapper<GLID>::IndexType;
    /**
     * @brief The IndexType matrix containning value type
     * 
     */
    using IndexValue = typename IndexType::value_type;
    /**
     * @brief A vector of FOV cell data type, the cell data type is actually chipimgproc::IdxRect, 
     *   so the type CellInfos is similar to std::vector<chipimgproc::IdxRect>.
     * @details The CellInfos is a vector containning one or multiple FOVs' cell data 
     *   which has same coordinates on the chip. If the cell position is in a none overlapping region of a FOV,
     *   then the CellInfos vector should contains only one cell data, but if the cell position is in a overlapping region,
     *   then the CellInfos vector should contains multiple cells data.
     */
    using CellInfos  = detail::CellInfos_<FLOAT>;
    /**
     * @brief The chip level cell container. The actual type of Tiles is like std::vector<std::vector<IdxRect>>
     * @details Given a (row, col) position, we can access not only one cell data.
     *   Here we notice that the cell data is the concept on FOV image, 
     *   but there are several overlapping regions between FOVs.
     *   In such case, a (row, col) position may contains multiple cells from different FOVs.
     *   The concept of Tile is to represent the single cell on the "chip" level, 
     *   and the Tiles is the container of Tile and will be access by the IndexType object.
     * 
     */
    using Tiles      = typename detail::TilesWrapper<FLOAT>::Tiles;
    /**
     * @brief A shortcut to represent "this" type
     * 
     */
    using This       = MultiTiledMat<FLOAT, GLID>;
    /**
     * @brief create MultiTiledMat
     *  
     * @param imgs          FOV gridding result, 
     *                      In general we use TiledMat::make_from_grid_res 
     *                      to create TiledMat from gridding::Result.
     * @param stats         Cell level statistic data, usually generate from margin process. 
     *                      Note that, the order should match the parameter imgs.
     * @param cell_st_pts   Cell level logical stitch points.
     *                      Note that, the order should match the parameter imgs.
     * @param fov_index     The FOV position IDs following the imgs order. 
     *                      Note that, the FOV position ID format is (x,y) not (r,c).
     */
    MultiTiledMat(
        const std::vector<TiledMat<GLID>>&        imgs,
        const std::vector<stat::Mats<FLOAT>>&     stats,
        const std::vector<cv::Point>&             cell_st_pts,
        const std::vector<cv::Point>&             fov_index = {}
    )
    : detail::IndexedRange<FLOAT, GLID>()
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
        (detail::IndexedRange<FLOAT, GLID>&)(*this) = 
            detail::IndexedRange<FLOAT, GLID>(
                detail::TilesWrapper<FLOAT>::tiles_,
                detail::IndexWrapper<GLID>::index_
            );
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
        // std::cout << "markers, before unique: " << std::endl;
        // for( auto& m : markers_ ) {
        //     std::cout << m << std::endl;
        // }
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

        // std::cout << "markers, after unique: " << std::endl;
        // for( auto& m : markers_ ) {
        //     std::cout << m << std::endl;
        // }

        // initial fov index
        if( !fov_index.empty() ) {
            int rs = 0;
            int cs = 0;
            for( auto& fov_p : fov_index ) {
                if( fov_p.x > cs ) cs = fov_p.x;
                if( fov_p.y > rs ) rs = fov_p.y;
            }
            fov_index_ = decltype(fov_index_)(rs+1, cs+1);
            int i = 0;
            for( auto& fov_p : fov_index ) {
                fov_index_(fov_p.y, fov_p.x) = i;
                i++;
            }
        }
        cell_st_pts_ = cell_st_pts;
    }
private:
    struct MinCVMean {
        FLOAT operator()( const CellInfos& cell_infos ) const {
            auto min_cv = std::numeric_limits<FLOAT>::max();
            FLOAT res = -1.0;
            if( cell_infos.size() <= 0 ) 
                throw std::runtime_error("BUG: no cell info found");
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res = ci.mean;
                }
            }
            if( res < 0 )  {
                res = cell_infos.at(0).mean;
            }
            return res;
        }
    };
    struct MinCVPixels {
        MinCVPixels(const MultiTiledMat& m)
        : mm_(m)
        {}
        cv::Mat operator()( const CellInfos& cell_infos ) const {
            auto min_cv = std::numeric_limits<FLOAT>::max();
            cv::Mat res;
            if( cell_infos.size() <= 0 ) 
                throw std::runtime_error("BUG: no cell info found");
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res = mm_.cali_imgs_.at(ci.img_idx).mat()(ci).clone();
                }
            }
            if(res.empty()) {
                auto& ci = cell_infos.at(0);
                res = mm_.cali_imgs_.at(ci.img_idx).mat()(ci).clone();
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
            bool res_ready = false;
            Result res;
            for(auto& ci : cell_infos) {
                if(min_cv > ci.cv ) {
                    min_cv = ci.cv;
                    res.pixels      = mm_.cali_imgs_.at(ci.img_idx).mat()(ci).clone();
                    res.cell_info   = ci;
                    res_ready       = true;
                }
            }
            if(!res_ready) {
                auto& ci        = cell_infos.at(0);
                res.pixels      = mm_.cali_imgs_.at(ci.img_idx).mat()(ci).clone();
                res.cell_info   = ci;
            }
            return res;
        }
        const MultiTiledMat& mm_;

    };
    static constexpr MinCVMean min_cv_mean_{};
public:
    /**
     * @brief The element access strategy. Get the mean value for each cell.
     *  For overlapping region, select the cell which has minimum CV.
     *  The functor return is a float point value 
     * 
     * @return const MinCVMean& Cell select functor.
     */
    const MinCVMean& min_cv_mean() const {
        return min_cv_mean_;
    }
    /**
     * @brief The element access strategy. Get the pixels(cv::Mat type) for each cell.
     *  For overlapping region, select the cell which has minimum CV.
     *  The functor return is a float point value 
     * 
     * @return MinCVPixels Cell select functor.
     */
    MinCVPixels min_cv_pixels() const {
        return MinCVPixels(*this);
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
    template<class CELL_INFOS_FUNC = decltype(min_cv_mean_)&>
    decltype(auto) at(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean_
    ) const {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class CELL_INFOS_FUNC = decltype(min_cv_mean_)&>
    decltype(auto) at(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean_
    ) {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class CELL_INFOS_FUNC = decltype(min_cv_mean_)&>
    decltype(auto) operator()(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean_
    ) const {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class CELL_INFOS_FUNC = decltype(min_cv_mean_)&>
    decltype(auto) operator()(
        std::uint32_t row, std::uint32_t col, 
        CELL_INFOS_FUNC&& cell_infos_func = min_cv_mean_
    ) {
        return at_impl(*this, row, col, FWD(cell_infos_func));
    }

    template<class FUNC = decltype(min_cv_mean_)&>
    cv::Mat dump(FUNC&& func = min_cv_mean_) const {
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
    auto get_fov_rows() {
        return fov_index_.rows;
    }
    auto get_fov_rows() const {
        return fov_index_.rows;
    }
    auto get_fov_cols() {
        return fov_index_.cols;
    }
    auto get_fov_cols() const {
        return fov_index_.cols;
    }

    const auto& cell_level_stitch_points() const {
        return cell_st_pts_;
    }

    auto& cell_level_stitch_points() {
        return cell_st_pts_;
    }
    const auto& cell_level_stitch_point(int x, int y) const {
        return cell_st_pts_.at(fov_index_(y, x));
    }

    const auto& cell_level_stitch_point(int x, int y) {
        return cell_st_pts_.at(fov_index_(y, x));
    }
private:
    template<
        class THIS__, 
        class CELL_INFOS_FUNC = decltype(min_cv_mean_)
    >
    static decltype(auto) at_impl(
        THIS__&             this_, 
        std::uint32_t       row, 
        std::uint32_t       col, 
        CELL_INFOS_FUNC&&   cell_infos_func = min_cv_mean_
    ) {
        auto&& cell_infos = this_.tiles_.at(
            this_.index_(row, col)
        );
        return cell_infos_func(FWD(cell_infos));
    }
    std::vector<GridRawImg<GLID>>      cali_imgs_   ;
    std::vector<cv::Rect>              markers_     ;
    cv::Mat_<std::uint16_t>            fov_index_   ;
    std::vector<cv::Point>             cell_st_pts_ ;
};


}