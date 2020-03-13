#pragma once
#include <range/v3/distance.hpp>
#include <algorithm>

namespace chipimgproc::utils {

template<class Rng>
double percentile(Rng&& rng, double p) {
    const double int_margin = 0.00001;
    auto size = ranges::distance(rng);
    auto i = (size - 1) * p * 0.01;
    int i_floor = int(i);
    auto tmp = i - i_floor; // tmp: [0, 1)
    std::nth_element(rng.begin(), rng.begin() + i_floor, rng.end());
    std::nth_element(rng.begin(), rng.begin() + i_floor + 1, rng.end());
    auto a = rng[i_floor];
    auto b = rng[i_floor + 1];
    if(tmp < int_margin) return a;
    if(tmp > (1.0 - int_margin)) return b;
    return a + ( (b - a) * tmp );
}

}