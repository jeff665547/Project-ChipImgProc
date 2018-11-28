/**
 *  @file    ChipImgProc/sharpness.h
 *  @brief   The sharpness compute utilities.
 */
#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc {
/**
 *  @brief Compute the sharpness of the image.
 *  @param m The input image.
 *  @return Sharpness score.
 */
double sharpness( const cv::Mat& m );

}