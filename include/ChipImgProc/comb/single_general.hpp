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
#include <ChipImgProc/algo/um2px_auto_scale.hpp>
#include <ChipImgProc/marker/roi_append.hpp>

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
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl_mask,
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
            candi_pats_cl_mask,
            candi_pats_px,
            candi_pats_px_mask
        ); // TODO: raw marker image required
    }
    void set_chip_cell_info(float h_um, float w_um, float sp_um) {
        cell_h_um_ = h_um;
        cell_w_um_ = w_um;
        space_um_ = sp_um;
    }
    void enable_um2px_r_auto_scale(float um2px_r) {
        um2px_r_detection_ = true;
        curr_um2px_r_ = um2px_r;
    }
    void disable_um2px_r_auto_scale(float um2px_r) {
        um2px_r_detection_ = false;
        curr_um2px_r_ = um2px_r;
    }
    void disable_um2px_r_auto_scale() {
        um2px_r_detection_ = false;
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
    template<class FUNC>
    void set_marker_seg_append_viewer(const FUNC& v) {
        v_marker_append_ = v;
    }
    void disable_background_fix(bool flag) {
        disable_bg_fix_ = flag;
    }
    float get_um2px_r() {
        if(curr_um2px_r_ < 0) 
            throw std::runtime_error("um to pixel rate not detected");
        return curr_um2px_r_;
    }
    auto get_marker_layout() {
        return marker_layout_;
    }
    void set_ref_rot_degree(const std::optional<float>& rot_degree) {
        ref_rot_degree_ = rot_degree;
    }

    /**
     *  @brief The main function of image process pipeline.
     *  @details See SingleGeneral.
     *  @param src Input image, currently only support 16 bit image.
     *  @param id A task tag for console viewing, useful for debug.
     *  @return A tuple of type std::tuple<bool, chipimgproc::TiledMat, chipimgproc::stat::Mats, float>, 
     *          which represent (image QC, image process result, statistic result, rotation angle in degree)
     */
    auto operator() (
        const cv::Mat& src, 
        const std::string& id = ""
    ) {
        std::function<void(const cv::Mat&)> func;
        *msg_ << "img id: " << id << std::endl;
        if(v_sample_)
            v_sample_(viewable(src));
        std::vector<marker::detection::MKRegion> marker_regs;
        float theta                 = 0;
        float theta_off             = 0;
        cv::Mat tmp                 = src.clone();
        int iteration_times         = 0;
        int iteration_max_times     = 6;
        int start_record_theta_time = 2;
        float theta_threshold       = 0.01;
        std::vector<float>       candidate_theta;
        std::vector<cv::Point>   low_score_marker_idx;
        // if has ref rotation degree, test all marker
        if(ref_rot_degree_) {
            // set best marker index
            std::vector<float>          test_thetas     ;
            std::vector<std::size_t>    test_thetas_i   ;
            for(
                std::size_t i = 0; 
                i < marker_layout_.get_single_pat_candi_num();
                i ++
            ) {
                auto marker_regs = marker_detection_(
                    static_cast<const cv::Mat_<std::uint16_t>&>(tmp), 
                    marker_layout_, 
                    chipimgproc::MatUnit::PX, 
                    i,
                    *msg_
                );
                marker::detection::filter_low_score_marker(marker_regs);
                auto test_theta = rot_estimator_(marker_regs, *msg_);
                test_thetas.push_back(test_theta);
                test_thetas_i.push_back(i);
            }
            std::sort(test_thetas_i.begin(), test_thetas_i.end(), [&test_thetas, this](
                auto&& a_i, auto&& b_i
            ){
                return std::abs(test_thetas[a_i] - ref_rot_degree_.value()) < 
                    std::abs(test_thetas[b_i] - ref_rot_degree_.value());
            });
            marker_layout_.set_single_pat_best_mk(test_thetas_i[0]);
            theta = ref_rot_degree_.value();
            rot_calibrator_(tmp, theta, v_rot_cali_res_);
        } else {
            // iterative rotation calibration
            do{
                auto marker_regs = marker_detection_(
                    static_cast<const cv::Mat_<std::uint16_t>&>(tmp), 
                    marker_layout_, 
                    chipimgproc::MatUnit::PX, 
                    0,
                    *msg_
                );
                low_score_marker_idx = 
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
        }
        // detect marker
        if(um2px_r_detection_) {
            if( cell_w_um_ < 0 ) throw std::runtime_error("um2px_r detection require cell micron info but not set");
            algo::Um2PxAutoScale auto_scaler(
                tmp, 
                cell_w_um_,
                cell_h_um_,
                space_um_
            );
            auto[best_um2px_r, score_mat] = auto_scaler.linear_steps(
                marker_layout_, curr_um2px_r_, 0.002, 7, 
                low_score_marker_idx, *msg_
            );
            curr_um2px_r_  = best_um2px_r;
            marker_regs    = marker::detection::reg_mat_no_rot.infer_marker_regions(
                score_mat, marker_layout_, MatUnit::PX, *msg_
            );
            um2px_r_detection_ = false; 
            // assume all chip images run in single process (which means object not destroied)
            // is use same reader scanned, so the um2px_r not re-detected by default.
            // um2px_r can be re-detected by manual invoke enable function.
        }
        else {
            // assume the marker is single pattern regular matrix layout
            marker::make_single_pattern_reg_mat_layout(
                marker_layout_,
                cell_w_um_,
                cell_h_um_,
                space_um_,
                curr_um2px_r_
            );
            marker_regs    = marker::detection::reg_mat_no_rot(
                tmp, marker_layout_, MatUnit::PX, 
                low_score_marker_idx, *msg_, v_marker_seg_
            );
        }
        auto grid_res   = gridder_(tmp, marker_layout_, marker_regs, *msg_, v_grid_res_);
        if(v_marker_append_) {
            auto marker_append_res = chipimgproc::marker::roi_append(
                tmp, marker_layout_, *msg_
            );
            v_marker_append_(marker_append_res);
        }
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
        auto [margin_res, bg_value] = [&]() {
            auto tiles = tiled_mat.get_tiles();
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
                auto bg_value = bgb::chunk_local_mean(
                    dirty_margin_res.stat_mats.mean,
                    grid_raw_img,   // int image -> float image
                    3, 3, 0.05,     // TODO: parameterize
                    marker_layout_, 
                    true, 
                    std::cout
                );
                std::cout << "after integer fix: " << std::endl;
                std::cout << "tiled_mat: " << std::endl;
                chipimgproc::info(std::cout, tiled_mat.get_cali_img());
                std::cout << std::endl;
                std::cout << "grid_raw_img: " << std::endl;
                chipimgproc::info(std::cout, grid_raw_img.mat());
                std::cout << std::endl;
                tiled_mat.get_cali_img() = grid_raw_img.mat();
                tiled_mat.get_tiles() = tiles;
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
                    std::move(margin_res), std::move(bg_value)
                );
            } else {
                auto bg_value = bgb::chunk_local_mean(
                    dirty_margin_res.stat_mats.mean,
                    grid_raw_img,   // int image -> float image
                    3, 3, 0.05,     // TODO: parameterize
                    marker_layout_, 
                    false, 
                    std::cout
                );
                return nucleona::make_tuple(
                    std::move(dirty_margin_res), std::move(bg_value)
                );
            }
        }();
        return nucleona::make_tuple(
            true,
            std::move(tiled_mat), 
            std::move(margin_res.stat_mats), 
            std::move(theta),
            std::move(bg_value)
        );
    }
  private:
    std::string               margin_method_     { "mid_seg" }                   ; // available algorithm: mid_seg, auto_min_cv
    float                     seg_rate_          { 0.6 }                         ;
    marker::Layout            marker_layout_                                     ;
    std::ostream*             msg_               { &nucleona::stream::null_out } ;
    bool                      disable_bg_fix_    { false }                       ;
    float                     curr_um2px_r_      {-1}                            ;
    bool                      um2px_r_detection_ {false}                         ;
    float                     cell_h_um_         {-1}                            ;
    float                     cell_w_um_         {-1}                            ;
    float                     space_um_          {-1}                            ;
    std::optional<float>      ref_rot_degree_    { 0}                            ;

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

    std::function<
        void(const cv::Mat&)
    >                         v_marker_append_   { nullptr }                     ;

    chipimgproc::marker::detection::RegMat       marker_detection_   ;
    chipimgproc::rotation::MarkerVec<FLOAT>      rot_estimator_      ;
    chipimgproc::rotation::Calibrate             rot_calibrator_     ;
    chipimgproc::rotation::Cache<FLOAT>          rot_cache_          ;
    chipimgproc::gridding::RegMat                gridder_            ;
    chipimgproc::Margin<FLOAT, GLID>             margin_             ;
    chipimgproc::roi::RegMatMarkerLayout         roi_bounder_        ;
};

}}