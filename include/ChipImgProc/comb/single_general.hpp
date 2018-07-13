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
#include <Nucleona/tuple.hpp>

namespace chipimgproc{ namespace comb{

template<
    class FLOAT = float, 
    class GLID  = std::uint16_t
>
struct SingleGeneral {

    using Gridline = GLID;
    using TiledMatT = TiledMat<Gridline>;

    void set_rot_theta_range( FLOAT min, FLOAT max ) {
        rot_min_theta_ = min;
        rot_max_theta_ = max;
    }
    void set_rot_estimate_steps(FLOAT steps) {
        rot_steps_ = steps;
    }
    void set_grid_max_invl(int invl) {
        grid_max_invl_ = invl;
    }
    void set_grid_support_img( const cv::Mat& mat) {
        grid_img_ = &mat;
    }
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
    void set_edges_viewer( const FUNC& v ) {
        v_edges_ = v;
    }
    template<class FUNC>
    void set_hough_tf_viewer(const FUNC& v) {
        v_hough_ = v;
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
    void set_roi_bin_viewer(const FUNC& v) {
        v_roi_bin_ = v;
    }
    template<class FUNC>
    void set_roi_res_viewer(const FUNC& v) {
        v_roi_res_ = v;
    }
    template<class FUNC>
    void set_roi_score_viewer(const FUNC& v) {
        v_roi_score_ = v;
    }
    
    auto operator() (const cv::Mat& src, const std::string& id = "") {
        *msg_ << "img id: " << id << std::endl;
        auto marker_regs = marker_detection_(
            static_cast<const cv::Mat_<std::uint16_t>&>(src), 
            marker_layout_, 
            chipimgproc::MatUnit::PX, 
            *msg_
        );

        bool has_grid_img_ = (grid_img_ != nullptr);
        rot_cache_.set_img_id(id);
        auto theta = [this, &src, has_grid_img_, &marker_regs]() {
            if(rot_cache_.has_cache()) {
                return rot_cache_.get_cache();
            } else {
                // auto v = rot_estimator_(
                //     src,
                //     has_grid_img_,
                //     has_grid_img_ ? *grid_img_ : cv::Mat(),
                //     rot_min_theta_,
                //     rot_max_theta_,
                //     rot_steps_,
                //     *msg_,
                //     v_sample_,
                //     v_edges_,
                //     v_hough_
                // );
                auto v = rot_estimator_(
                    marker_regs, *msg_
                );
                rot_cache_.save_cache(v);
                return v;
            }
        }();
        cv::Mat tmp = src.clone();
        rot_calibrator_(
            tmp,
            theta,
            v_rot_cali_res_
        );
        auto grid_res   = gridder_(tmp, grid_max_invl_, *msg_, v_grid_res_);
        auto tiled_mat  = TiledMat<>::make_from_grid_res(grid_res, tmp);
        auto margin_res = margin_(
                            margin_method_,
                            margin::Param<GLID> {
                                seg_rate_, 
                                &tiled_mat,
                                v_margin_res_
                            }
                          );
        auto roi_qc     = roi_bounder_(
                            marker_layout_, 
                            margin_res.stat_mats,
                            tiled_mat,
                            *msg_,
                            v_roi_bin_,
                            v_roi_score_,
                            v_roi_res_
                          );
        return nucleona::make_tuple(
            std::move(roi_qc), 
            std::move(tiled_mat), 
            std::move(margin_res.stat_mats), 
            std::move(theta)
        );
    }
  private:
    FLOAT                     rot_min_theta_     {  87 }                         ; 
    FLOAT                     rot_max_theta_     {  93 }                         ; 
    FLOAT                     rot_steps_         { 800 }                         ;
    FLOAT                     grid_max_invl_     {  35 }                         ;
    const cv::Mat*            grid_img_          { nullptr }                     ;
    std::string               margin_method_     { "mid_seg" }                   ;
    float                     seg_rate_          { 0.6 }                         ;
    marker::Layout            marker_layout_                                     ;
    std::ostream*             msg_               { &nucleona::stream::null_out } ;

    std::function<
        void(const cv::Mat&)
    >                         v_sample_          { nullptr }                     ;
    std::function<
        void(const cv::Mat&)
    >                         v_edges_           { nullptr }                     ;
    std::function<
        void(const cv::Mat&)
    >                         v_hough_           { nullptr }                     ;
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
    >                         v_roi_bin_         { nullptr }                     ;
    std::function<
        void(const cv::Mat&, int, int)
    >                         v_roi_score_       { nullptr }                     ;
    std::function<
        void(const cv::Mat&)
    >                         v_roi_res_         { nullptr }                     ;

    chipimgproc::marker::detection::RegMat       marker_detection_   ;
    chipimgproc::rotation::MarkerVec<FLOAT>      rot_estimator_      ;
    chipimgproc::rotation::Calibrate             rot_calibrator_     ;
    chipimgproc::rotation::Cache<FLOAT>          rot_cache_          ;
    chipimgproc::Gridding<FLOAT>                 gridder_            ;
    chipimgproc::Margin<FLOAT, GLID>             margin_             ;
    chipimgproc::roi::RegMatMarkerLayout         roi_bounder_        ;

};

}}