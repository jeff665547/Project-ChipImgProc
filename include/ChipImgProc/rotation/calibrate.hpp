/**
 *    @file      ChipImgProc/calibrate.hpp
 *    @author    Alex Lee
 *    @brief     Rotate the image by given angle.
 */
#pragma once
#include <cmath>
#include <random>
#include <ChipImgProc/utils.h>

namespace chipimgproc{ namespace rotation{

/**
 *    @brief     Rotate the image by given angle.
 *    @details Input the angle and image, the function rotate the image in-place.
 *    @details Detail information can see here @ref improc_image_rotation
 */
struct Calibrate
{
    public:
        
        /**
         *    @brief Rotate the image by given angle.
         *    @param in_src   The input image.
         *    @param theta    The input rotate angle.
         *    @param v_result Show the rotate result.
         */
        template<class FLOAT>
        auto operator()( 
              cv::Mat& in_src
            , FLOAT theta
            , const std::function<void(const cv::Mat&)>& v_result = nullptr
        )
        {
            auto& src = in_src;
            cv::Point2f center(src.cols >> 1, src.rows >> 1);
            auto mat = cv::getRotationMatrix2D(center, theta, 1.0);
            cv::warpAffine(src, src, mat, src.size());
            if(v_result) {
                v_result(src);
            }
        }
};
}}
