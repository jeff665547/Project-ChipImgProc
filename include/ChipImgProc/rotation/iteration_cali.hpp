#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc::rotation {

template<class Float, class RotEst, class RotCali>
struct IterationCali {
    IterationCali(
        int             max_time, 
        Float           theta_threshold,
        const RotEst&   rot_est,
        const RotCali&  rot_cali
    )
    : max_time_         (max_time)
    , theta_threshold_  (theta_threshold)
    , rot_est_          (rot_est)
    , rot_cali_         (rot_cali)
    {}

    auto operator()(
        const cv::Mat& src, 
        std::ostream& logger = nucleona::stream::null_out
    ) const {
        const int start_record_theta_time = 2;

        cv::Mat tmp = src.clone();
        Float theta_off, theta;
        int iter_times = 0;
        std::vector<Float> candi_theta;
        do {
            theta_off = rot_est_(tmp);
            theta += theta_off;
            tmp = src.clone();
            rot_cali_(tmp, theta);
            if(iter_times > start_record_theta_time) {
                candi_theta.push_back(theta);
            }
            iter_times++;
            if(iter_times >= max_time_) break;
        } while(std::abs(theta_off) > theta_threshold_);
        if( std::abs(theta_off) > theta_threshold_ ) {
            logger << "theta not converge\n";
            Float sum = 0;
            for(auto&& t : candi_theta) {
                sum += t;
            }
            theta = sum / candi_theta.size();
            // tmp = src.clone();
            // rot_cali_(tmp, theta);
        }
        return theta;
    }
private:
    int         max_time_           ;
    Float       theta_threshold_    ;
    RotEst      rot_est_            ;
    RotCali     rot_cali_           ;
};

template<class RotEst, class RotCali, class Float = float>
auto make_iteration_cali(
    const RotEst&   rot_est, 
    const RotCali&  rot_cali,
    int             max_time        = 6,
    Float           theta_threshold = 0.01
) {
    return IterationCali<Float, RotEst, RotCali>(
        max_time, theta_threshold,
        rot_est, rot_cali
    );
}

}