#pragma once
#include <fitpackpp/BSplineSurface.h>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/utils/mat_to_vec.hpp>
#include <ChipImgProc/utils/percentile.hpp>
#include <cmath>
#include <ChipImgProc/algo/fitpack.h>
#include <Nucleona/range.hpp>

namespace chipimgproc::bgb {

constexpr struct BSpline {
    auto operator()(
        cv::Mat _mat, 
        const std::vector<double>& q, 
        const double reduce_factor = -1,
        const int k = 3,
        std::optional<double> s = std::nullopt
    ) const {
        using T = std::uint16_t;
        if(q.size() != 2) throw std::runtime_error("BSpline: parameter error, q must be size of 2");

        cv::Mat mat;
        if(reduce_factor < 0) {
            mat = _mat;
        } else {
            cv::resize(_mat, mat, cv::Size(0,0), reduce_factor, reduce_factor);
        }

        // percentile
        auto vec = chipimgproc::utils::mat_to_vec<T>(mat);
        auto l = chipimgproc::utils::percentile(vec, q[0]);
        auto u = chipimgproc::utils::percentile(vec, q[1]);

        // where
        std::vector<double> x;
        std::vector<double> y;
        std::vector<double> log2_v;
        for(int i = 0; i < mat.rows; i ++ ) {
            for(int j = 0; j < mat.cols; j ++ ) {
                auto v = mat.template at<T>(i, j);
                if(v >= l && v <= u) {
                    x.push_back(i);
                    y.push_back(j);
                    log2_v.push_back(std::log2(v));
                }
            }
        }
        
        // bisplrep
        auto [tx, ty, c] = chipimgproc::algo::fitpack::bisplrep(x, y, log2_v, k, s);
        x = ranges::to_vector(
            ranges::view::transform(
                nucleona::range::irange_0(mat.rows), [](auto v){ return double(v); }
            )
        );
        y = ranges::to_vector(
            ranges::view::transform(
                nucleona::range::irange_0(mat.cols), [](auto v){ return double(v); }
            )
        );
        auto log_surf = chipimgproc::algo::fitpack::bisplev(tx, ty, c, k, x, y);
        cv::Mat_<double> surf, _surf(log_surf.rows, log_surf.cols, log_surf.type());
        _surf.forEach([&log_surf](double& v, const int* pos){
            const auto& r = pos[0];
            const auto& c = pos[1];
            v = std::pow(2, log_surf(r, c));
        });
        log_surf.release();

        if(reduce_factor < 0) {
            surf = _surf;
        } else {
            cv::resize(_surf, surf, cv::Size(_mat.cols, _mat.rows));
        }
        // surf = cv::min(_mat, surf);
        return surf;

        // cv::Mat_<std::uint16_t> res = _mat.clone();
        // res.forEach([&surf](std::uint16_t& v, const int* pos){
        //     const auto& r = pos[0];
        //     const auto& c = pos[1];
        //     v = std::round(std::max(0.0, v - surf(r, c)));
        // });
        // return res;
    }
} bspline;

}