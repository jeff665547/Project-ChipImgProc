#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <Nucleona/tuple.hpp>

namespace chipimgproc {

template<class Float>
struct MakeStatByConvolution {

    auto operator()(
        cv::Mat mat, int conv_w, int conv_h
    ) const {
        auto x_mean = mean(mat, conv_w, conv_h);
        auto x_2 = mat.mul(mat);
        auto x_2_mean = mean(x_2, conv_w, conv_h);
        cv::Mat var = x_2_mean - x_mean;
        cv::Mat sd(var.size(), var.type());
        cv::sqrt(var, sd);
        return nucleona::make_tuple(
            std::move(x_mean),
            std::move(sd)
        );
    }

private:
    auto mean(
        cv::Mat mat, int conv_w, int conv_h
    ) const {
        cv::Mat_<Float> dst;
        cv::Mat kern(conv_h, conv_w, dst.type());
        kern.setTo(1.0 / (conv_w * conv_h))
        cv::filter2D(mat, dst, dst.depth(), kern);
        return dst;
    } 
};

}