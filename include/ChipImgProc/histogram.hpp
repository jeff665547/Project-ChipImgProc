#pragma once
#include <vector>
#include <ChipImgProc/utils.h>
namespace chipimgproc {

constexpr struct Histogram {
    using ResultBase = std::vector<std::pair<
        float, std::size_t 
    >>;
    struct Result : public ResultBase
    {
        using Base = ResultBase;
        using Base::Base;
        float lbound;
        float ubound;
        float bin_size;
    };
    template<class DataRng>
    Result operator()(DataRng&& mat, std::size_t bin_num, double ubound, double lbound, bool boundary_check = false) const {
        Result res(bin_num);
        auto bin_size = (ubound - lbound) / bin_num;
        for(std::size_t i = 0; i < res.size(); i ++ ) {
            res.at(i).first = bin_size * i + bin_size / 2 + lbound;
        }
        auto idx = [&](auto v) {
            if(v >= lbound && v < ubound)
                return (std::size_t)std::floor((v - lbound) / bin_size);
            else 
                throw std::out_of_range("index failed");
        };
        for(auto&& v : mat) {
            std::size_t id;
            try {
                id = idx(v);
                res.at(id).second += 1;
            } catch(const std::out_of_range& e) {
                if(boundary_check) throw;
            }
        }
        // res[0].second = 0;
        res.lbound = lbound;
        res.ubound = ubound;
        res.bin_size = bin_size;
        return res;
    }
    template<class T>
    auto get_ubound() const {
        if constexpr(std::is_floating_point_v<T>) {
            return 1.0;
        } else {
            return std::numeric_limits<T>::max();
        }
    }
    template<class T>
    auto get_lbound() const {
        if constexpr(std::is_floating_point_v<T>) {
            return 0;
        } else {
            return std::numeric_limits<T>::min();
        }
    }

    template<class T>
    auto get_bin_size() const {
        if constexpr(std::is_same_v<T, std::uint8_t>) {
            return 1;
        } else {
            return 130;
        }
    }
    
    template<class Value>
    auto operator()(const cv::Mat_<Value>& mat, std::size_t bin_num = 256, bool boundary_check = false) const {
        double min, max;
        cv::minMaxLoc(mat, &min, &max, nullptr, nullptr);
        auto res = operator()(mat, 
            bin_num,
            max, min, 
            boundary_check 
        );
        if(res.back().second == 0) {
            res.pop_back();
        }
        return res;
    }
} histogram;

}