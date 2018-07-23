#pragma once
#include <ChipImgProc/utils.h>
#include <map>
#include <boost/math/constants/constants.hpp>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc{ namespace rotation{

template<class FLOAT = float>
struct GridPointInfer {

    using DirPointsGroup = std::map<int, std::vector<cv::Point>>;

private:

    template<class T>
    auto mean( std::vector<T>& d, std::ostream& out ) const {
        std::sort(d.begin(), d.end());
        auto mid = d.at(d.size() / 2);
        out << "angle mid: " << mid << std::endl;
        T sum = 0;
        int n = 0;
        for( const auto& v : d ) {
            auto diff = std::abs(v - mid);
            if( diff > 1 ) {
                out << "bad angle: " << v << std::endl;
            } else {
                sum += v;
                n ++;
            }
        }
        auto res = sum / n;
        out << "total used angles: " << n << std::endl;
        out << "angles mean: " << res << std::endl;

        return res;
    }
    auto angle(const cv::Point& a, const cv::Point& b) const {
        auto dot = (a.x * b.x) + (a.y * b.y);
        auto det = (a.x * b.y) - (a.y * b.x);
        auto rd  = std::atan2(det, dot);
        return rd * 180 / boost::math::constants::pi<FLOAT>();
    }
    template<class FUNC>
    auto angle_estimate(
        DirPointsGroup              d_group,
        FUNC&&                      dir_compare,
        const cv::Point&            dir_vec
    ) const {
        std::vector<FLOAT> degrees;
        for( auto&& p : d_group ) {
            std::sort(p.second.begin(), p.second.end(), FWD(dir_compare));
            auto vec = p.second.back() - p.second.front();
            degrees.push_back(angle(dir_vec, vec));
        }
        return degrees;
    }
    auto horizontal_estimate(DirPointsGroup& x_group) const {
        return angle_estimate(x_group, [](const auto& a, const auto& b){
            return a.y < b.y;
        }, cv::Point(0,1));
    }
    auto vertical_estimate(DirPointsGroup& y_group) const {
        return angle_estimate(y_group, [](const auto& a, const auto& b){
            return a.x < b.x;
        }, cv::Point(1,0));
    }

public:
    auto operator()( 
        DirPointsGroup& x_group, 
        DirPointsGroup& y_group,
        std::ostream&   logger = nucleona::stream::null_out 
    ) const {
        auto h_thetas = horizontal_estimate(x_group);
        auto v_thetas = vertical_estimate(y_group);
        std::vector<FLOAT> thetas = h_thetas;
        thetas.insert(thetas.end(),v_thetas.begin(), v_thetas.end());
        return mean(thetas, logger);
    }    
};

}}