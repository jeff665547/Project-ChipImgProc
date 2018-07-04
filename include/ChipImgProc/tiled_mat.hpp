#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range/irange.hpp>
namespace chipimgproc{

template<
    class GLID = std::uint16_t
>
struct TiledMat
{
    using IndexType = cv::Mat_<std::int32_t>;
    TiledMat(int rows, int cols)
    : index_(rows, cols)
    , cali_img_()
    , tiles_()
    , gl_x_()
    , gl_y_()
    {
        tiles_.reserve(rows * cols);
    }
    cv::Rect& tile_at(int r, int c) {
        return tiles_.at(index_(r, c));
    }
    const cv::Rect& tile_at(int r, int c) const {
        return tiles_.at(index_(r, c));
    }
    cv::Mat tile_img_at(int r, int c) const {
        return cali_img_(tile_at(r, c));
    }
    auto& get_tiles() {
        return tiles_;
    }
    const auto& get_tiles() const {
        return tiles_;
    }
    cv::Mat& get_cali_img() {
        return cali_img_;
    }
    const cv::Mat& get_cali_img() const {
        return cali_img_;
    }
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
        func(debug_img);
    }
    // decltype(auto) operator()( int x, int y ) {
    //     return index_(x, y);
    // }
    auto cols() const {
        return index_.cols;
    }
    auto rows() const {
        return index_.rows;
    }
    auto cols() {
        return index_.cols;
    }
    auto rows() {
        return index_.rows;
    }
    auto& glx() { return gl_x_; }
    auto& gly() { return gl_y_; }
    const auto& glx() const { return gl_x_; }
    const auto& gly() const { return gl_y_; }
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
    cv::Mat index() const {
        return index_.clone();
    }
    cv::Rect get_image_roi() const {
        auto w = gl_x_.back() - gl_x_.front();
        auto h = gl_y_.back() - gl_y_.front();
        return cv::Rect(
            gl_x_.at(0), gl_y_.at(0),
            w, h
        );
    }
    cv::Mat get_roi_image() const {
        return cali_img_(get_image_roi());
    }
    template<class GRID_RES>
    static TiledMat<GLID> make_from_grid_res(GRID_RES& grid_res, cv::Mat& rot_cali_img ) {
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
        return tm;
    }
    
private:
    IndexType index_;
    cv::Mat cali_img_;
    std::vector<cv::Rect> tiles_;
    std::vector<GLID> gl_x_;
    std::vector<GLID> gl_y_;
};
}