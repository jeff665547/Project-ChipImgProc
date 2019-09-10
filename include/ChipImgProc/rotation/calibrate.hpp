/**
 * @file      calibrate.hpp
 * @author    Alex Lee
 * @brief     Rotate the image by given angle.
 * 
 */
#pragma once
#include <cmath>
#include <random>
#include <ChipImgProc/utils.h>

namespace chipimgproc{ namespace rotation{

/**
 *    @brief     This functor aims to rotate the image with a given rotation angle in degrees.
 *    @details   Input the angle and image, the function rotates the image in-place. 
 */
struct Calibrate
{
    public:
        
        /**
         *    @brief Rotate the image with a given angle.
         *    @param in_src    The input image.
         *    @param theta     The input rotation angle. Positive values mean counter-clockwise rotation
         *                     (the coordinate origin is assumed to be the top-left corner).
         *    @param v_result  Show the rotation result.
         */
        template<class FLOAT>
        auto operator()( 
              cv::Mat& in_src
            , FLOAT theta
            , const std::function<void(const cv::Mat&)>& v_result = nullptr
        ) const
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
