#include <algorithm>
#include <numeric>
#include <vector>
#include <stdexcept>
#include <opencv2/core.hpp>

//TODO add comments
namespace chipimgproc {

template <class T>
class PartialPercentile {
  public:
    template <class SELECTOR = std::vector<int32_t>>
    double operator()(const cv::Mat& values, const double q, SELECTOR&& indices = SELECTOR()) {
        return (*this)(cv::Mat_<T>(values), q, std::forward<SELECTOR>(indices)); 
    }
    template <class SELECTOR = std::vector<int32_t>>
    double operator()(const cv::Mat_<T>& values, const double q, SELECTOR&& indices = SELECTOR()) {
        if (indices.empty()) {
            std::vector<int32_t> indexes(values.total());
            std::iota(indexes.begin(), indexes.end(), 0);
            return this->calculate(values, q, std::move(indexes));
        } else {
            return this->calculate(values, q, std::forward<SELECTOR>(indices));
        }
    }
    template <class SELECTOR = std::vector<int32_t>>
    double operator()(const std::vector<T>& values, const double q, SELECTOR&& indices = SELECTOR()) {
        auto&& getter = [&values](auto& i){ return values[i]; };
        if (indices.empty()) {
            std::vector<int32_t> indexes(values.size());
            std::iota(indexes.begin(), indexes.end(), 0);
            return this->calculate(std::move(getter), q, std::move(indexes));
        } else {
            return this->calculate(std::move(getter), q, std::forward<SELECTOR>(indices));
        }
    }

  private:
    template <class CONTAINER, class SELECTOR>
    double calculate(CONTAINER&& values, const double q, SELECTOR&& indices) {
        double result;
        if (indices.size() > 0) {
            auto n = indices.size() - 1;
            auto f = q * n;
            auto i = static_cast<int32_t>(f);
            for (auto j = i; j <= i + (i != n); ++j) {
                std::nth_element(
                    indices.begin()
                  , indices.begin() + j
                  , indices.end()
                  , [&values](auto&& lhs, auto&& rhs) {
                        return values(lhs) < values(rhs);
                    }
                );
                if (j == i)
                    result = values(indices[j]);
                else
                    result += (values(indices[j]) - result) * (f - i);
            }
        } else {
            throw std::length_error("indices.size() < 1");
        }
        return result;
    }
};
}
