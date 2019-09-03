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
     * @brief The marker position index along X direction, 
     *        for 3*3 markers on image, the x_i will be [0, 1, 2]
     */
    int x_i;
    /**
     * @brief The marker position index along Y direction, 
     *        for 3*3 markers on image, the y_i will be [0, 1, 2]
     */
    int y_i;
    /**
     * @brief The marker matching score.
     *        This value will be set after marker detection.
     * 
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
     * @brief Print the marker region information to given stream.
     * 
     * @param out out stream object.
     */
    void info(std::ostream& out) {
        out << "(" << this->x << "," << this->y << ")"
            << "[" << this->width << "," << this->height << "]"
            << "(" << x_i << "," << y_i << ")"
            << std::endl
        ;
    }
    /**
     * @brief Group marker region with the same x_i.
     * 
     * @tparam FUNC     Function type. Deduced, can be 
     * @param regs      The marker regions.
     * @param func      The marker region to user specifed object tranform.
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
     * @brief Group marker region with the same y_i.
     * 
     * @tparam FUNC     Function type. Deduced, can be 
     * @param regs      The marker regions.
     * @param func      The marker region to user specifed object tranform.
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