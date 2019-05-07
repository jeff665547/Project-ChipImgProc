/**
 *  @file    ChipImgProc/tiled_mat.hpp
 *  @author  Chia-Hua Chang
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range.hpp>
namespace chipimgproc{


/**
 *  @brief   Image matrix include after gridding, include grid tile informations.
 *  @tparam  The integer type use to score the grid line position, 
 *           this depend on the image size.
 */
template<
    class GLID = std::uint16_t
>
struct TiledMat
{
    using IndexType = cv::Mat_<std::int32_t>;
    /**
     *  @brief        Tiled matrix constructor.
     *  @param rows   Grid level row number.
     *  @param cols   Grid level column number.
     */
    TiledMat(int rows, int cols)
    : index_(rows, cols)
    , cali_img_()
    , tiles_()
    , gl_x_()
    , gl_y_()
    {
        tiles_.reserve(rows * cols);
    }
    /**
     *  @brief Get the grid level tile region on image.
     *  @param r The grid level row position.
     *  @param c The grid level column position.
     *  @return A bound rectangle of related tile position on image.
     */
    cv::Rect& tile_at(int r, int c) {
        return tiles_.at(index_(r, c));
    }
    /**
     *  @brief Get the grid level tile region on image.
     *  @param r The grid level row position.
     *  @param c The grid level column position.
     *  @return A bound rectangle of related tile position on image.
     */
    const cv::Rect& tile_at(int r, int c) const {
        return tiles_.at(index_(r, c));
    }
    /**
     *  @brief Get the grid level image of selected tile.
     *  @param r The grid level row position.
     *  @param c The grid level column position.
     *  @return A partial image of related tile position on original image.
     */
    cv::Mat tile_img_at(int r, int c) const {
        return cali_img_(tile_at(r, c));
    }
    /**
     * @brief Get all tiles.
     * @return A reference tile list.
     */
    auto& get_tiles() {
        return tiles_;
    }
    /**
     * @brief Get all tiles.
     * @return A reference tile list.
     */
    const auto& get_tiles() const {
        return tiles_;
    }
    /**
     * @brief Get image.
     * @return A reference to the image.
     */
    cv::Mat& get_cali_img() {
        return cali_img_;
    }
    /**
     * @brief Get image.
     * @return A reference to the image.
     */
    const cv::Mat& get_cali_img() const {
        return cali_img_;
    }
    /**
     * @brief view The grid image, useful for debugging.
     */
    template<class FUNC>
    void view(FUNC&& func) {
        cv::Mat_<std::uint16_t> debug_img = viewable(cali_img_);
        auto color = 65536/2;
        for (auto tile: tiles_)
        {
            tile.width  += 1;
            tile.height += 1;
            cv::rectangle(debug_img, tile, color);
        }
        for(auto l : gl_x_) {
            cv::line(debug_img, 
                cv::Point(l, 0), cv::Point(l, debug_img.rows), 
                color, 1
            );
        }
        for(auto l : gl_y_) {
            cv::line(debug_img, 
                cv::Point(0, l), cv::Point(debug_img.cols, l), 
                color, 1
            );
        }
        func(debug_img);
    }
    /**
     * @brief The grid level column number.
     */
    auto cols() const {
        return index_.cols;
    }
    /**
     * @brief The grid level column number.
     */
    auto rows() const {
        return index_.rows;
    }
    /**
     * @brief The grid level column number.
     */
    auto cols() {
        return index_.cols;
    }
    /**
     * @brief The grid level column number.
     */
    auto rows() {
        return index_.rows;
    }
    /**
     * @brief The grid line along the x axis.
     */
    auto& glx() { return gl_x_; }
    /**
     * @brief The grid line along the y axis.
     */
    auto& gly() { return gl_y_; }
    /**
     * @brief The grid line along the x axis.
     */
    const auto& glx() const { return gl_x_; }
    /**
     * @brief The grid line along the y axis.
     */
    const auto& gly() const { return gl_y_; }
    /**
     * @brief The grid level region of interest.
     * @param r A grid level rectangle region.
     */
    void roi(const cv::Rect& r ) {
        index_ = index_(r);
        decltype(gl_x_) tmp_x(
            gl_x_.begin() + r.x, 
            gl_x_.begin() + r.x + r.width + 1
        );
        gl_x_.swap(tmp_x);
        decltype(gl_y_) tmp_y(
            gl_y_.begin() + r.y, 
            gl_y_.begin() + r.y + r.height + 1
        );
        gl_y_.swap(tmp_y);
    }
    /**
     * @brief The internal tile index.
     * @details The index store the mapping of grid position and tile list.
     *         For example: index()(10, 5) -> 201, 
     *         means the get_tiles()[201] is at grid level position (10, 5).
     */
    cv::Mat index() const {
        return index_.clone();
    }
    /**
     * @brief Get the grid line covered region on image.
     * @return The bound rectangle of image.
     */
    cv::Rect get_image_roi() const {
        auto w = gl_x_.back() - gl_x_.front();
        auto h = gl_y_.back() - gl_y_.front();
        return cv::Rect(
            gl_x_.at(0), gl_y_.at(0),
            w, h
        );
    }
    /**
     * @brief Get the grid line covered region on image.
     * @return The bound rectangle of image.
     */
    cv::Mat get_roi_image() const {
        return cali_img_(get_image_roi());
    }
    /**
     * @brief Create TiledMat object form gridding result.
     * @param grid_res      A parameter pack include the gridding algorithm output, 
     *                      basically the chipimgproc::gridding::Result type
     * @param rot_cali_img  Image after rotation calibration.
     * @param mk_layout     Chip depended marker layout on rot_cali_img.
     */
    template<class GRID_RES>
    static TiledMat<GLID> make_from_grid_res(
        GRID_RES&               grid_res        , 
        cv::Mat&                rot_cali_img    ,
        const marker::Layout&   mk_layout
    ) {
        // TODO: consider forward parameter
        namespace nr = nucleona::range;
        if(
            grid_res.feature_rows * (std::uint32_t)grid_res.feature_cols 
            != grid_res.tiles.size()
        ) {
            throw std::runtime_error("BUG: grid tile number and feature cols * rows not matched");
        }
        if( grid_res.tiles.size() > std::numeric_limits<std::int32_t>::max() ) {
            throw std::runtime_error("too many grid tiles, tile matrix only support up to " + 
                std::to_string(std::numeric_limits<std::int32_t>::max()) + " the require is " +
                std::to_string(grid_res.tiles.size())
            );
        }
        TiledMat<GLID> tm(
            grid_res.feature_rows,
            grid_res.feature_cols
        );
        int cols = tm.index_.cols;
        int rows = tm.index_.rows;
        for( decltype(grid_res.feature_rows) r = 0; r < grid_res.feature_rows; r ++ ) {
            for( decltype(grid_res.feature_cols) c = 0; c < grid_res.feature_cols; c ++ ) {
                std::int32_t tid = (r * (std::int32_t)grid_res.feature_cols) + c;
                tm.index_(r, c) = tid;
            }
        }
        tm.cali_img_ = rot_cali_img;
        tm.tiles_ = grid_res.tiles;
        tm.gl_x_.reserve(grid_res.gl_x.size());
        for(auto v : grid_res.gl_x ) {
            tm.gl_x_.push_back((GLID)v);
        }
        tm.gl_y_.reserve(grid_res.gl_y.size());
        for(auto v : grid_res.gl_y ) {
            tm.gl_y_.push_back((GLID)v);
        }

        for(auto& mk_des : mk_layout.mks ) {
            tm.markers_.push_back(
                cv::Rect(
                    mk_des.get_pos_cl().x,
                    mk_des.get_pos_cl().y,
                    mk_layout.get_marker_width_cl(),
                    mk_layout.get_marker_height_cl()
                )
            );
        }
        return tm;
    }
    /**
     * @brief Get marker list.
     */
    const std::vector<cv::Rect>& markers() const {
        return markers_;
    }
    /**
     * @brief Get marker list.
     */
    std::vector<cv::Rect>& markers() {
        return markers_;
    }
    
private:
    IndexType               index_      ;
    cv::Mat                 cali_img_   ;
    std::vector<cv::Rect>   tiles_      ;
    std::vector<GLID>       gl_x_       ;
    std::vector<GLID>       gl_y_       ;
    std::vector<cv::Rect>   markers_    ;
};
}