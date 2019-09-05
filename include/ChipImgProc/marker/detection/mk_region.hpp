/**
 * @file mk_region.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::marker::detection::MKRegion
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <iostream>
#include <map>
namespace chipimgproc{ namespace marker{ namespace detection{

/**
 * @brief The marker region on any level of image.
 * 
 */
struct MKRegion : public cv::Rect {
    /**
     * @brief The marker location index along X-axis.
     *        For example, given a 3-by-3 grid of markers within the image,
     *        the value of x_i will be 0, 1 or 2.
     */
    int x_i;

    /**
     * @brief The marker location index along Y-axis.
     *        For example, given a 3-by-3 grid of markers within the image,
     *        the value of y_i will be 0, 1 or 2.
     */
    int y_i;

    /**
     * @brief The quality score of the marker recognition.
     */
    double score;

    /**
     * @brief Group the object by the marker position index.
     * 
     * @tparam T The grouped object type
     */
    template<class T>
    using Group = std::map<int, std::vector<T>>;

    /**
     * @brief Export the content of marker region to the given ostream object.
     * 
     * @param out output stream object.
     */
    void info(std::ostream& out) {
        out << "(" << this->x << "," << this->y << ")"
            << "[" << this->width << "," << this->height << "]"
            << "(" << x_i << "," << y_i << ")"
            << std::endl
        ;
    }

    /**
     * @brief Group the marker regions by the value of x_i.
     * 
     * @tparam FUNC     Function type. Deduced, can be 
     * @param regs      A set of marker regions.
     * @param func      A transfomation to grouped marker regions.
     * @return auto     Deduced, depend on FUNC result type. The pattern is Group<T>.
     */
    template<class FUNC>
    static auto x_group(const std::vector<MKRegion>& regs, FUNC&& func) {
        Group<decltype(func(std::declval<MKRegion>()))> group;
        for(auto&& r : regs ) {
            group[r.x_i].push_back(func(r));
        }
        return group;
    }

    /**
     * @brief Group the marker regions by the value of y_i.
     * 
     * @tparam FUNC     Function type. Deduced, can be 
     * @param regs      A set of marker regions.
     * @param func      A transfomation to grouped marker regions.
     * @return auto     Deduced, depend on FUNC result type. The pattern is Group<T>.
     */
    template<class FUNC>
    static auto y_group(const std::vector<MKRegion>& regs, FUNC&& func) {
        Group<decltype(func(std::declval<MKRegion>()))> group;
        for(auto&& r : regs ) {
            group[r.y_i].push_back(func(r));
        }
        return group;
    }
    /**
     * @brief Group marker region into marker location with the same x_i.
     * 
     * @param regs Input marker regions
     * @return auto Deduced, usually std::vector<Group<cv::Point>>
     */
    static auto x_group_points( const std::vector<MKRegion>& regs ) {
        return x_group(regs, [](const MKRegion& mk_reg){
            return cv::Point(mk_reg.x, mk_reg.y);
        });
    }
    /**
     * @brief Group marker region into marker location with the same y_i.
     * 
     * @param regs Input marker regions
     * @return auto Deduced, usually std::vector<Group<cv::Point>>
     */
    static auto y_group_points( const std::vector<MKRegion>& regs ) {
        return y_group(regs, [](const MKRegion& mk_reg){
            return cv::Point(mk_reg.x, mk_reg.y);
        });
    }
};
}}}