#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range/irange.hpp>
namespace chipimgproc{

template<
    class GLID = std::uint16_t
>
// using TID = std::int32_t;
// using GLID = std::uint16_t;
struct TiledMat
{
    TiledMat(int rows, int cols)
    : index_(rows, cols, CV_32SC1)
    , cali_img_()
    , tiles_()
    , gl_x_()
    , gl_y_()
    {
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        tiles_.reserve(rows * cols);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
    }
    cv::Rect& tile_at(int x, int y) {
        return tiles_.at(index_.template at<std::int32_t>(x, y));
    }
    cv::Mat tile_img_at(int x, int y) {
        return cali_img_(tile_at(x, y));
    }
    auto& get_tiles() {
        return tiles_;
    }
    cv::Mat& get_cali_img() {
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
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        TiledMat<GLID> tm(
            grid_res.feature_rows,
            grid_res.feature_cols
        );
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        int cols = tm.index_.cols;
        int rows = tm.index_.rows;
        for( decltype(grid_res.feature_rows) r = 0; r < grid_res.feature_rows; r ++ ) {
            for( decltype(grid_res.feature_cols) c = 0; c < grid_res.feature_cols; c ++ ) {
                std::int32_t tid = (r * (std::int32_t)grid_res.feature_cols) + c;
                tm.index_.template at<std::int32_t>(r, c) = tid;
            }
        }
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        info(std::cout, rot_cali_img);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        tm.cali_img_ = rot_cali_img;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        std::cout << "grid_res.tiles.size(): " << grid_res.tiles.size() << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        tm.tiles_ = grid_res.tiles;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        std::cout << "gl_x.size(): " << grid_res.gl_x.size() << std::endl;
        std::cout << "gl_y.size(): " << grid_res.gl_y.size() << std::endl;
        tm.gl_x_.reserve(grid_res.gl_x.size());
        for(auto v : grid_res.gl_x ) {
            tm.gl_x_.push_back((GLID)v);
        }
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        tm.gl_y_.reserve(grid_res.gl_y.size());
        for(auto v : grid_res.gl_y ) {
            tm.gl_y_.push_back((GLID)v);
        }
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        return tm;
    }
    
private:
    cv::Mat index_;
    cv::Mat cali_img_;
    std::vector<cv::Rect> tiles_;
    std::vector<GLID> gl_x_;
    std::vector<GLID> gl_y_;
};
}