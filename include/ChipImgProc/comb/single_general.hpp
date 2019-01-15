/**
 * @file ChipImgProc/comb/single_general.hpp
 * @brief The combined chip image process algorthm for single FOV.
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
#include <ChipImgProc/marker/detection/infer.hpp>
#include <ChipImgProc/marker/detection/filter_low_score_marker.hpp>
#include <ChipImgProc/marker/detection/reg_mat_no_rot.hpp>
namespace chipimgproc{ namespace comb{

/**
 *  @brief Chip image process pipelin for single FOV image.
 *  @tparam FLOAT The float point type used during image process, depend on user application.
 *  @tparam GLID  The integer type used during image prcoess, depend on image size.
 *  
 *  @details The image process pipeline follows these steps: <BR>
 *  1. Marker detection. See: chipimgproc::marker::detection::RegMat <BR>
 *  2. Rotation estimate. See: chipimgproc::rotation::MarkerVec <BR>
 *  3. Rotation calibration. See: chipimgproc::rotation::Calibrate <BR>
 *  4. Marker detection again. See: chipimgproc::marker::detection::RegMat <BR>
 *  5. Grid line estimate and generate. See: chipimgproc::Gridding <BR>
 *  6. Each grid cell margin, use to prevent probe defect. See: chipimgproc::Margin <BR>
 *  7. Background process. See: chipimgproc::bgb::ChunkLocalMean <BR>
 *  9. Each grid cell margin again, because the background process may change the best margin position.
 * 
 *  Basic usage can see unit test: <BR> 
 *  @snippet ChipImgProc/comb/single_general_test.cpp usage
 */
template<
    class FLOAT = float, 
    class GLID  = std::uint16_t
>
struct SingleGeneral {

    using Gridline = GLID;
    using TiledMatT = TiledMat<Gridline>;
    /**
     *  @brief Set marker layout used in next image process task.
     *  @details set_marker_layout must call before firset call operator invoked, otherwise the behavior is undefined.
     *  @param candi_pats_cl   Candidate marker pattern in grid cell level.
     *  @param candi_pats_px   Candidate marker pattern in pixel level.
     *  @param rows            Marker numbers on row.
     *  @param cols            Marker numbers on column.
     *  @param invl_x_cl       Distance between marker along x direction in grid cell level.
     *  @param invl_y_cl       Distance between marker along y direction in grid cell level.
     *  @param invl_x_px       Distance between marker along x direction in pixel level.
     *  @param invl_y_px       Distance between marker along y direction in pixel level.
     *  @param min_p           The most left top marker position on image.
     */
    void set_marker_layout( 
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px_mask,
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
        marker_layout_.set_single_mk_pat( 
            candi_pats_cl, 
            candi_pats_px,
            candi_pats_px_mask
        ); // TODO: raw marker image required
    }
    /**
     *  @brief Set marker layout used in next image process task.
     *  @details set_marker_layout must call before firset call operator invoked, otherwise the behavior is undefined.
     *  @param mkl marker layout object.
     */
    void set_marker_layout( const marker::Layout& mkl ) {
        marker_layout_ = mkl;
    }
    /**
     *  @brief Set verbose logger used in next image process task.
     *  @details If the set_logger is not set the default behavior is no verbose.
     */
    void set_logger( std::ostream& out) {
        msg_ = &out;
    }
    /**
     *  @brief Set margin method, there are two margin method: "mid_seg: and "auto_min_cv" 
     *  @details The mid_seg can see: chipimgproc::margin::MidSeg. <BR> 
     *          The auto_min_cv can see: chipimgproc::AutoMinCV. <BR>
     *          By default, the method is "mid_seg"
     *  @param method Can be "mid_seg" or "auto_min_cv"
     */
    void set_margin_method ( const std::string& method ) {
        margin_method_ = method;
    }
    /**
     *  @brief Set margin segment rate.
     *  @param rate The segment rate, range is [0, 1).
     */
    void set_seg_rate( float rate ) {
        seg_rate_ = rate;
    }
    /**
     *  @brief Set image viewer. Useful for debug.
     *  @param v Viewer function callback, function type is void(const cv::Mat&). 
     *           v function will be invoked at begin of image process.
     */
    template<class FUNC> 
    void set_sample_viewer( const FUNC& v ) {
        v_sample_ = v;
    }
    /**
     *  @brief Set rotation calibration image viewer. Useful for debug.
     *  @param v Viewer function call back, function type is void(const cv::Mat&). 
     *           v function will invoked after image rotation calibration step done.
     */
    template<class FUNC>
    void set_rot_cali_viewer(const FUNC& v) {
        v_rot_cali_res_ = v;
    }
    /**
     *  @brief Set grid result image viewer. Useful for debug.
     *  @param v Viewer function call back, function type is void(const cv::Mat&). 
     *           v function will invoked after image gridding step done.
     */
    template<class FUNC>
    void set_grid_res_viewer(const FUNC& v ) {
        v_grid_res_ = v;
    }
    /**
     *  @brief Set margin result image viewer. Useful for debug.
     *  @param v Viewer function call back, function type is void(const cv::Mat&). 
     *           v function will invoked after image margin step done.
     */
    template<class FUNC>
    void set_margin_res_viewer(const FUNC& v) {
        v_margin_res_ = v;
    }
    /**
     *  @brief Set marker detecion image viewer. Useful for debug.
     *  @param v Viewer function call back, function type is void(const cv::Mat&). 
     *           v function will invoked after image marker detection step done.
     */
    template<class FUNC>
    void set_marker_seg_viewer(const FUNC& v) {
        v_marker_seg_ = v;
    }
    void disable_background_fix(bool flag) {
        disable_bg_fix_ = flag;
    }
    /**
     *  @brief The main function of image process pipeline.
     *  @details See SingleGeneral.
     *  @param src Input image, currently only support 16 bit image.
     *  @param id A task tag for console viewing, useful for debug.
     *  @return A tuple of type std::tuple<bool, chipimgproc::TiledMat, chipimgproc::stat::Mats, float>, 
     *          which represent (image QC, image process result, statistic result, rotation angle in degree)
     */
    auto operator() (const cv::Mat& src, const std::string& id = "") {
        std::function<void(const cv::Mat&)> func;
        *msg_ << "img id: " << id << std::endl;
        if(v_sample_)
            v_sample_(viewable(src));
        std::vector<marker::detection::MKRegion> marker_regs;
        float theta = 0;
        float theta_off = 0;
        cv::Mat tmp = src.clone();
        int iteration_times = 0;
        int iteration_max_times = 6;
        int start_record_theta_time = 2;
        float theta_threshold = 0.01;
        std::vector<float> candidate_theta;
        do{
            auto marker_regs = marker_detection_(
                static_cast<const cv::Mat_<std::uint16_t>&>(tmp), 
                marker_layout_, 
                chipimgproc::MatUnit::PX, 
                *msg_
            );
            marker::detection::filter_low_score_marker(marker_regs);
            theta_off = rot_estimator_(marker_regs, *msg_);
            theta += theta_off;
            tmp = src.clone();
            rot_calibrator_(
                tmp,
                theta,
                v_rot_cali_res_
            );
            if( iteration_times > start_record_theta_time ) {
                candidate_theta.push_back(theta);
            }
            iteration_times ++;
            if( iteration_times >= iteration_max_times) break;
        } while(std::abs(theta_off) > theta_threshold);
        if( std::abs(theta_off) > theta_threshold ) {
            *msg_ << "theta not convergence" << std::endl;
            float sum = 0;
            for(auto&& t : candidate_theta) {
                sum += t;
            }
            theta = sum / candidate_theta.size();
            tmp = src.clone();
            rot_calibrator_(
                tmp,
                theta,
                v_rot_cali_res_
            );
        }
        // detect marker
        marker_regs = marker::detection::reg_mat_no_rot(
            tmp, marker_layout_, MatUnit::PX, *msg_, v_marker_seg_
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
        auto margin_res = [&]() {
            auto dirty_margin_res = margin_( // TODO: cv mean or direct segmentation ?
                                margin_method_,
                                margin::Param<GLID> {
                                    seg_rate_, 
                                    &tiled_mat,
                                    disable_bg_fix_, 
                                    v_margin_res_
                                }
                              );
            if(!disable_bg_fix_) {
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
                {
                    grid_raw_img.mat().convertTo(
                        tiled_mat.get_cali_img(), CV_16UC1, 1.0
                    );
                }
                return margin_res;
            } else {
                return dirty_margin_res;
            }
        }();
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
    bool                      disable_bg_fix_    { false }                       ;

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