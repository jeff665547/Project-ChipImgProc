/**
 * @file iteration_cali.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::rotation::IterationCali 
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc::rotation {

/**
 * @brief Iterative search the rotation angle. To use this algorithm, 
 *        we suggest call the factory function: 
 *        chipimgproc::rotation::make_iteration_cali()
 * 
 * @details This algorithm takes 4 parameters: 
 * rotation estimate function, rotation calibrate function, 
 * max iteration time and theta threshold
 * 
 * And run with following steps:
 * 
 * 1. Call the rotation estimate function and record the returning rotation angle.
 * 2. Call the rotation calibrate function to update the image.
 * 3. Increment the iteration times.
 * 4. Check iteration times. If more than max iteration times, 
 *    break the iteration loop and return the mean of the estimated angles.
 * 5. Check the last two estimate angles, 
 *    if the difference is less than theta threshold, loop break and 
 *    direct return the current estimated angle.
 * 6. Goto 1.
 * 
 * TODO: example code
 * 
 * @tparam Float    The float point type use to run the algorithm.
 * @tparam RotEst   The rotation angle estimation function.
 *                  The function symbol is Float(const cv::Mat&, ...).
 * @tparam RotCali  The rotation angle calibration function.
 *                  The function symbol is Float(const cv::Mat&, Float, ...).
 * 
 */
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
    /**
     * @brief Call operator, input image and additional estimate and calibrate arguments.
     * 
     * @tparam EstArgs Deduced, a tuple type contains the estimate function additional parameters. 
     * @tparam CaliArgs Deduced, a tuple type contains the calibrate function additional parameters. 
     * 
     * @param src           Input image.
     * @param est_args      Estimate function additional parameters wrap by std::tuple 
     * @param cali_args     Calibrate function additional parameters wrap by std::tuple
     * @return auto         Deduced, same as template parameter Float. The final result angle.
     */
    template<
        class EstArgs = std::tuple<>, 
        class CaliArgs = std::tuple<>
    > auto operator()(
        const cv::Mat& src, 
        const EstArgs& est_args = {},
        const CaliArgs& cali_args = {}
    ) const {
        const int start_record_theta_time = 2;

        cv::Mat tmp = src.clone();
        Float theta_off, theta;
        int iter_times = 0;
        std::vector<Float> candi_theta;
        auto rot_est = [this, &tmp](auto&&... args) {
            return rot_est_(tmp, FWD(args)...);
        };
        auto rot_cali = [this, &tmp, &theta](auto&&... args) {
            return rot_cali_(tmp, theta, FWD(args)...);
        };
        do {
            theta_off = std::apply(rot_est, est_args);
            theta += theta_off;
            tmp = src.clone();
            std::apply(rot_cali, cali_args);
            if(iter_times > start_record_theta_time) {
                candi_theta.push_back(theta);
            }
            iter_times++;
            if(iter_times >= max_time_) break;
        } while(std::abs(theta_off) > theta_threshold_);
        if( std::abs(theta_off) > theta_threshold_ ) {
            // logger << "theta not converge\n";
            Float sum = 0;
            for(auto&& t : candi_theta) {
                sum += t;
            }
            theta = sum / candi_theta.size();
        }
        return theta;
    }
private:
    int         max_time_           ;
    Float       theta_threshold_    ;
    RotEst      rot_est_            ;
    RotCali     rot_cali_           ;
};
/**
 * @brief The maker function of chipimgproc::rotation::IterationCali type.
 * 
 * @details @copydetails chipimgproc::rotation::IterationCali
 * 
 * @tparam RotEst   Function/Functor type of rot_est.
 *                  The function symbol is Float(const cv::Mat&, ...).
 * @tparam RotCali  Function/Functor type of rot_cali.
 *                  The function symbol is Float(const cv::Mat&, Float, ...).
 * @tparam Float    The float point type use to run the algorithm.
 * 
 * @param rot_est           Rotation estimation function/functor.
 * @param rot_cali          Rotation calibration function/functor.
 * @param max_time          The max iteration calibrate times.
 *                          If the iteration time reach the max time
 *                          but the threshold not yet reached, 
 *                          the iteration process still exit.
 *                          By default is 6
 * @param theta_threshold   The rotation angle threshold. 
 * @return auto  Deduced, The chipimgproc::rotation::IterationCali functor type.
 */
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