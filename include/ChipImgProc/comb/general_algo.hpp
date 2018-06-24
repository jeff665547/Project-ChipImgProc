#include <ChipImgProc/rotation/estimate.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <ChipImgProc/gridding.hpp>
#include <ChipImgProc/margin.hpp>
#include <ChipImgProc/tiled_mat.hpp>
namespace chipimgproc{ namespace comb{

template<class FLOAT>
struct GeneralAlgo {
    // using FLOAT  = float;
    // using ResultBase = typename chipimgproc::Gridding<FLOAT>::Result;
    // struct Result : public ResultBase {
    //     Result(const typename ResultBase::Result& base, const cv::Mat& _rot_cali_res )
    //     : ResultBase(base)
    //     , rot_cali_res(_rot_cali_res)
    //     {}
    //     cv::Mat rot_cali_res;
    // };

    void set_rot_theta_range( FLOAT min, FLOAT max ) {
        rot_min_theta_ = min;
        rot_max_theta_ = max;
    }
    void set_rot_estimate_steps(FLOAT steps) {
        rot_steps_ = steps;
    }
    void set_grid_max_intvl(int intvl) {
        grid_max_intvl_ = intvl;
    }
    void set_grid_support_img( const cv::Mat& mat) {
        grid_img_ = &mat;
    }
    void set_logger( std::ostream& out) {
        msg_ = &out;
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
    
    auto operator() (const cv::Mat& src) {
        bool has_grid_img_ = (grid_img_ != nullptr);
        // auto theta = rot_estimator_(
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
        cv::Mat tmp = src.clone();
        rot_calibrator_(
            tmp,
            0.607353,
            v_rot_cali_res_
        );
        auto grid_res   = gridder_(tmp, grid_max_intvl_, *msg_, v_grid_res_);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        auto tiled_mat  = TiledMat<>::make_from_grid_res(grid_res, tmp);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        auto margin_res = auto_min_cv_(tiled_mat, window_width, window_height, v_margin_res_);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;

        // if(mk_layout.dist_form != MarkerLayout::DistForm::uni_mat) {
        //     throw std::runtime_error("")
        // }


        // Result res( grid_res, tmp );
        // return res;
    }
  private:
    FLOAT                     rot_min_theta_     {  87 }                         ; 
    FLOAT                     rot_max_theta_     {  93 }                         ; 
    FLOAT                     rot_steps_         { 800 }                         ;
    FLOAT                     grid_max_intvl_    {  35 }                         ;
    const cv::Mat*            grid_img_          { nullptr }                     ;
    std::int32_t              window_width       {  20 }                         ;
    std::int32_t              window_height      {  20 }                         ;
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


    chipimgproc::rotation::Estimate<FLOAT>  rot_estimator_      ;
    chipimgproc::rotation::Calibrate        rot_calibrator_     ;
    chipimgproc::Gridding<FLOAT>            gridder_            ;
    chipimgproc::margin::AutoMinCV          auto_min_cv_        ;

};

}}