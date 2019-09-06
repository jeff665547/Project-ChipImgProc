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
 * @brief This class inherits from the class cv:Rect,
 *        which records the width and height of the shape, and the xy coordinate of top-left corner,
 *        with additional marker location index and matching quality score.
 *        This class can be applied to any integer scaling level of image such as
 *        integral-pixel-level, cell-level and marker-level scales,
 *        except micron-level scale.
 */
struct MKRegion : public cv::Rect {
    /**
     * @brief the marker location index along X-axis.
     *        For example, given a 3-by-3 grid of markers within the image,
     *        the value of x_i will be 0, 1 or 2.
     */
    int x_i;

    /**
     * @brief the marker location index along Y-axis.
     *        For example, given a 3-by-3 grid of markers within the image,
     *        the value of y_i will be 0, 1 or 2.
     */
    int y_i;

    /**
     * @brief the quality score of the marker recognition.
     */
    double score;

    /**
     * @brief an std::map for grouping objects in std::vector by the marker loctation index.
     * 
     * @tparam T the grouped object type
     */
    template<class T>
    using Group = std::map<int, std::vector<T>>;

    /**
     * @brief Export the content to the given ostream object.
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
     * @tparam FUNC     deduced, it can be any value returning function type
     * @param regs      a set of marker regions
     * @param func      a transfomation to grouped marker regions.
     * @return auto     deduced, the return type Group<T> depends on result type of FUNC.
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
     * @tparam FUNC     deduced, it can be any value returning function type
     * @param regs      a set of marker regions.
     * @param func      a transfomation to grouped marker regions.
     * @return auto     deduced, the return type Group<T> depends on result type of FUNC.
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
     * @param regs      the input marker regions
     * @return auto     deduced, the return type is std::vector<Group<cv::Point>>
     */
    static auto x_group_points( const std::vector<MKRegion>& regs ) {
        return x_group(regs, [](const MKRegion& mk_reg){
            return cv::Point(mk_reg.x, mk_reg.y);
        });
    }
    /**
     * @brief Group marker region into marker location with the same y_i.
     * 
     * @param regs      the input marker regions
     * @return auto     deduced, the return type is std::vector<Group<cv::Point>>
     */
    static auto y_group_points( const std::vector<MKRegion>& regs ) {
        return y_group(regs, [](const MKRegion& mk_reg){
            return cv::Point(mk_reg.x, mk_reg.y);
        });
    }
};
}}}