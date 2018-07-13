#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <boost/math/constants/constants.hpp>
#include <map>
#include <vector>
namespace chipimgproc { namespace rotation {

template<class FLOAT>
struct MarkerVec {

private:
    template<class T>
    auto mean( const std::vector<T>& d ) const {
        T sum = 0;
        for(auto& v : d) {
            sum += v;
        }
        return sum / d.size();
    }
    auto angle(const cv::Point& a, const cv::Point& b) const {
        auto dot = (a.x * b.x) + (a.y * b.y);
        auto det = (a.x * b.y) - (a.y * b.x);
        auto rd  = std::atan2(det, dot);
        return rd * 180 / boost::math::constants::pi<FLOAT>();
    }
    using DirPointsGroup = std::map<int, std::vector<cv::Point>>;
    auto x_y_group( const std::vector<marker::detection::MKRegion>& set ) const {
        DirPointsGroup x_group, y_group;
        for(auto&& mk_r : set ) {
            x_group[mk_r.x_i].push_back(cv::Point(mk_r.x, mk_r.y));
            y_group[mk_r.y_i].push_back(cv::Point(mk_r.x, mk_r.y));
        }
        return nucleona::make_tuple(
            std::move(x_group), std::move(y_group)
        );
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
        const std::vector<
            marker::detection::MKRegion
        >&                                      mk_regions                              ,
        std::ostream&                           logger     = nucleona::stream::null_out
    ) const {
        auto[x_group, y_group] = x_y_group(mk_regions);
        auto h_thetas = horizontal_estimate(x_group);
        auto v_thetas = vertical_estimate(y_group);
        std::vector<FLOAT> thetas = h_thetas;
        thetas.insert(thetas.end(),v_thetas.begin(), v_thetas.end());
        return mean(thetas);
    }
private:
    Calibrate rotator_;
};

}}