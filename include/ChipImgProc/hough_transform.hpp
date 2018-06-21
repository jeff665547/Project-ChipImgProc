#pragma once
#include <vector>
#include <ChipImgProc/utils.h>
// #include <CCD/para_thread_pool/para_thread_pool.hpp>
// #include <CCD/utility/tune_scope.hpp>
#include <Nucleona/parallel/thread_pool.hpp>
#include <Nucleona/range/irange.hpp>
#include <iostream>
namespace chipimgproc
{
template<class FLOAT>
struct HoughTransform
{
    struct UnitVector
    {
        FLOAT theta;
        FLOAT cos_theta;
        FLOAT sin_theta;

        UnitVector(FLOAT theta)
        : theta(theta)
        , cos_theta(std::cos(theta * CV_PI / 180.0))
        , sin_theta(std::sin(theta * CV_PI / 180.0))
        {}

        FLOAT rho(const int32_t x, const int32_t y) const
        {
            return x * cos_theta + y * sin_theta;
        }
    };
    static FLOAT rho(const std::int32_t x, const std::int32_t y, const FLOAT theta)
    {
        return x * std::cos(theta * CV_PI / 180.0)
             + y * std::sin(theta * CV_PI / 180.0);
    }
    HoughTransform(FLOAT tmin, FLOAT tmax, FLOAT steps )
    : tmin_ ( tmin  )
    , tmax_ ( tmax  )
    , tstep_( (tmax - tmin )/steps)
    {
        for (auto theta = tmin; theta < tmax; theta += tstep_)
            unitvecs_.emplace_back(theta);
    }

    const auto& unitvecs()
    {
        return unitvecs_;
    }

    auto operator()( cv::Mat img )
    {
        const auto cols = img.cols;
        const auto rows = img.rows;
        const auto tmid = std::atan2(rows, cols) * 180.0 / CV_PI;
        const auto rmin = std::floor(
            (tmax_ > 90.0)? rho(cols, 0, tmax_): 0.0
        );
        // const auto rmin = -50;
        const FLOAT rmax = std::ceil(
            (tmax_ < tmid)? rho(cols, rows, tmax_)
          : (tmin_ < tmid)? std::sqrt(rows * rows + cols * cols)
          : (tmin_ < 90.0)? rho(cols, rows, tmin_)
          :                rho(0   , rows, tmin_)
        );

        cv::Mat_<std::atomic<std::uint16_t>> hist = cv::Mat_<std::atomic<std::uint16_t>>::zeros(unitvecs_.size(), rmax - rmin + 1);
        chipimgproc::info(std::cout, hist);
        const auto thread_num = 4;
        {
            auto thread_pool = nucleona::parallel::make_thread_pool( thread_num );
            auto segsize = img.rows / thread_num;
            for ( int seg_beg = 0; seg_beg < img.rows; seg_beg += segsize) {
                segsize = std::min(segsize, img.rows - seg_beg);
                int seg_end = seg_beg + segsize;            
                thread_pool.job_post([ seg_beg, seg_end, &img, this, &hist, &rmin ]() mutable
                {
                    for ( int r = seg_beg; r < seg_end; r ++ )
                        for (auto c = 0; c != img.cols; ++c)
                            if (img.template at<uint8_t>(r, c) > 127) // and (std::rand() & 0x4) == 0)
                                for (auto& u: unitvecs_ )
                                {
                                    auto hi = std::round((u.theta     - tmin_) / tstep_);
                                    auto hj = std::round((u.rho(c, r) - rmin)          );
                                    auto v = hist( hi, hj ).load( std::memory_order_acquire );
                                    hist(hi, hj).compare_exchange_weak( v, v + 1
                                        , std::memory_order_release
                                        , std::memory_order_relaxed 
                                    );
                                }
                });
            }
            thread_pool.flush();
        }
        
        return hist;
    
    }
private:
    std::vector<UnitVector> unitvecs_;
    FLOAT tmin_, tmax_, tstep_;
};

void hough_transform_dummy();

}
