#pragma once
#include <vector>
#include <ChipImgProc/utils.h>
namespace chipimgproc {

constexpr struct Histogram {
    template<class DataRng, class Value>
    std::vector<
        std::pair<
            float, std::size_t 
        >
    > operator()(DataRng&& mat, std::size_t bin_size, Value ubound, Value lbound, bool boundary_check = false) const {
        std::vector<std::pair<float, std::size_t>> res(
            (std::size_t)std::ceil((ubound - lbound) / bin_size), {0, 0}
        );
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
    auto operator()(const cv::Mat_<Value>& mat, double ratio = 0.001, bool boundary_check = false) const {
        double min, max;
        cv::minMaxLoc(mat, &min, &max, nullptr, nullptr);
        auto bin_size = (Value)std::ceil((max - min) * ratio);
        return operator()(mat, bin_size, 
            (Value)std::ceil(max), 
            (Value)std::floor(min), 
            boundary_check 
        );
    }
} histogram;

}