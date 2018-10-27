/**
 * @file ChipImgProc/comb/single_general.hpp
 * @brief The combined chip image process algorthm.
 * @author Chia-Hua Chang
 */
#pragma once
#include <ChipImgProc/rotation/line_detection.hpp>
#include <ChipImgProc/marker/detection/reg_mat.hpp>
#include <ChipImgProc/rotation/marker_vec.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding.hpp>
#include <ChipImgProc/margin.hpp>
#include <ChipImgProc/tiled_mat.hpp>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/roi/reg_mat_marker_layout.hpp>
#include <ChipImgProc/rotation/cache.hpp>
#include <ChipImgProc/gridding/reg_mat.hpp>
#include <ChipImgProc/bgb/chunk_local_mean.hpp>
#include <Nucleona/tuple.hpp>

namespace chipimgproc{ namespace comb{

/**
 *  @brief Chip image process pipeline of single FOV image.
 *  @tparam FLOAT The float point type used during image process, depend on user application.
 *  @tparam GLID  The integer type used during image prcoess, depend on image size.
 *  
 *  @defail
 */
template<
    class FLOAT = float, 
    class GLID  = std::uint16_t
>
struct SingleGeneral {

    using Gridline = GLID;
    using TiledMatT = TiledMat<Gridline>;

    void set_marker_layout( 
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        int rows = 3, 
        int cols = 3,
        std::uint32_t invl_x_cl = 37, 
        std::uint32_t invl_y_cl = 37,
        std::uint32_t invl_x_px = 1091, // can get this value from micron to pixel
        std::uint32_t invl_y_px = 1091,
        const cv::Point& min_p = {0, 0}
    ) {
        marker_layout_.set_reg_mat_dist(
            rows,  cols,
            min_p,
            invl_x_cl, invl_y_cl,
            invl_x_px, invl_y_px
        );
        marker_layout_.set_single_mk_pat( candi_pats_cl, candi_pats_px ); // TODO: raw marker image required
    }
    void set_marker_layout( const marker::Layout& mkl ) {
        marker_layout_ = mkl;
    }
    void set_logger( std::ostream& out) {
        msg_ = &out;
    }
    void set_margin_method ( const std::string& method ) {
        margin_method_ = method;
    }
    void set_seg_rate( float rate ) {
        seg_rate_ = rate;
    }
    template<class FUNC> 
    void set_sample_viewer( const FUNC& v ) {
        v_sample_ = v;
    }
    template<class FUNC>
    void set_rot_cali_viewer(const FUNC& v) {
        v_rot_cali_res_ = v;
    }
    template<class FUNC>
    void set_grid_res_viewer(const FUNC& v ) {
        v_grid_res_ = v;
    }
    template<class FUNC>
    void set_margin_res_viewer(const FUNC& v) {
        v_margin_res_ = v;
    }
    template<class FUNC>
    void set_marker_seg_viewer(const FUNC& v) {
        v_marker_seg_ = v;
    }
    auto operator() (const cv::Mat& src, const std::string& id = "") {
        std::function<void(const cv::Mat&)> func;
        *msg_ << "img id: " << id << std::endl;
        if(v_sample_)
            v_sample_(viewable(src));
        // detect marker
        auto marker_regs = marker_detection_(
            static_cast<const cv::Mat_<std::uint16_t>&>(src), 
            marker_layout_, 
            chipimgproc::MatUnit::PX, 
            *msg_,
            nullptr,
            func,
            v_marker_seg_
        );
        auto theta = rot_estimator_(marker_regs, *msg_);
        cv::Mat tmp = src.clone();
        rot_calibrator_(
            tmp,
            theta,
            v_rot_cali_res_
        );
        marker_regs = marker_detection_(
            static_cast<const cv::Mat_<std::uint16_t>&>(tmp), 
            marker_layout_, MatUnit::PX, *msg_
        );
        auto grid_res   = gridder_(tmp, marker_layout_, marker_regs, *msg_, v_grid_res_);
        auto tiled_mat  = TiledMat<>::make_from_grid_res(
            grid_res, tmp, marker_layout_
        );
        GridRawImg<> grid_raw_img(
            tmp,
            grid_res.gl_x, 
            grid_res.gl_y
        );
        // basic gridding done, start calibrate matrix content
        // generate dirty mean, no background calibration
        auto dirty_margin_res = margin_( // TODO: cv mean or direct segmentation ?
                            margin_method_,
                            margin::Param<GLID> {
                                seg_rate_, 
                                &tiled_mat,
                                false, 
                                v_margin_res_
                            }
                          );
        std::cout << "before bg fix: " << std::endl;
        std::cout << "tiled_mat: " << std::endl;
        chipimgproc::info(std::cout, tiled_mat.get_cali_img());
        std::cout << std::endl;
        std::cout << "grid_raw_img: " << std::endl;
        chipimgproc::info(std::cout, grid_raw_img.mat());
        std::cout << std::endl;
        // start background calibration
        bgb::chunk_local_mean(
            dirty_margin_res.stat_mats.mean,
            grid_raw_img,   // int image -> float image
            3, 3, 0.05,     // TODO: parameterize
            marker_layout_, 
            std::cout
        );
        {
            grid_raw_img.mat().convertTo(
                tiled_mat.get_cali_img(), CV_16UC1, 1.0
            );
            // tiled_mat.view([](const cv::Mat_<std::uint16_t>& mat){
            //     cv::imwrite("debug_bg_cali.tiff", mat);
            // });
        }
        std::cout << "after integer fix: " << std::endl;
        std::cout << "tiled_mat: " << std::endl;
        chipimgproc::info(std::cout, tiled_mat.get_cali_img());
        std::cout << std::endl;
        std::cout << "grid_raw_img: " << std::endl;
        chipimgproc::info(std::cout, grid_raw_img.mat());
        std::cout << std::endl;
        auto margin_res = margin_(
            margin_method_,
            margin::Param<GLID> {
                seg_rate_, 
                &tiled_mat,
                true,
                v_margin_res_
            }
        );
        return nucleona::make_tuple(
            true,
            std::move(tiled_mat), 
            std::move(margin_res.stat_mats), 
            std::move(theta)
        );
    }
  private:
    std::string               margin_method_     { "mid_seg" }                   ; // available algorithm: mid_seg, auto_min_cv
    float                     seg_rate_          { 0.6 }                         ;
    marker::Layout            marker_layout_                                     ;
    std::ostream*             msg_               { &nucleona::stream::null_out } ;

    std::function<
        void(const cv::Mat&)
    >                         v_sample_          { nullptr }                     ;
    std::function<
        void(const cv::Mat&)
    >                         v_rot_cali_res_    { nullptr }                     ;
    std::function<
        void(const cv::Mat&)
    >                         v_grid_res_        { nullptr }                     ;
    std::function<
        void(const cv::Mat&)
    >                         v_margin_res_      { nullptr }                     ;

    std::function<
        void(const cv::Mat&)
    >                         v_marker_seg_      { nullptr }                     ;

    chipimgproc::marker::detection::RegMat       marker_detection_   ;
    chipimgproc::rotation::MarkerVec<FLOAT>      rot_estimator_      ;
    chipimgproc::rotation::Calibrate             rot_calibrator_     ;
    chipimgproc::rotation::Cache<FLOAT>          rot_cache_          ;
    chipimgproc::gridding::RegMat                gridder_            ;
    chipimgproc::Margin<FLOAT, GLID>             margin_             ;
    chipimgproc::roi::RegMatMarkerLayout         roi_bounder_        ;

};

}}