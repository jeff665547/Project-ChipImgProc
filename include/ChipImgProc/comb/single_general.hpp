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
#include <Nucleona/tuple.hpp>

namespace chipimgproc{ namespace comb{

// using FLOAT = float;
// using GLID = std::uint16_t;
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
    
    auto operator() (const cv::Mat& src, const std::string& id = "") {
        *msg_ << "img id: " << id << std::endl;
        if(v_sample_)
            v_sample_(viewable(src));
        auto marker_regs = marker_detection_(
            static_cast<const cv::Mat_<std::uint16_t>&>(src), 
            marker_layout_, 
            chipimgproc::MatUnit::PX, 
            *msg_
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
        auto margin_res = margin_(
                            margin_method_,
                            margin::Param<GLID> {
                                seg_rate_, 
                                &tiled_mat,
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

    chipimgproc::marker::detection::RegMat       marker_detection_   ;
    chipimgproc::rotation::MarkerVec<FLOAT>      rot_estimator_      ;
    chipimgproc::rotation::Calibrate             rot_calibrator_     ;
    chipimgproc::rotation::Cache<FLOAT>          rot_cache_          ;
    chipimgproc::gridding::RegMat                gridder_            ;
    chipimgproc::Margin<FLOAT, GLID>             margin_             ;
    chipimgproc::roi::RegMatMarkerLayout         roi_bounder_        ;

};

}}