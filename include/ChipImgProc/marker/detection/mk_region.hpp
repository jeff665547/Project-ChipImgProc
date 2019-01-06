#pragma once
#include <ChipImgProc/utils.h>
#include <iostream>
#include <map>
namespace chipimgproc{ namespace marker{ namespace detection{
struct MKRegion : public cv::Rect {
    int x_i;
    int y_i;
    double score;

    template<class T>
    using Group = std::map<int, std::vector<T>>;

    void info(std::ostream& out) {
        out << "(" << this->x << "," << this->y << ")"
            << "[" << this->width << "," << this->height << "]"
            << "(" << x_i << "," << y_i << ")"
            << std::endl
        ;
    }
    template<class FUNC>
    static auto x_group(const std::vector<MKRegion>& regs, FUNC&& func) {
        Group<decltype(func(std::declval<MKRegion>()))> group;
        for(auto&& r : regs ) {
            group[r.x_i].push_back(func(r));
        }
        return group;
    }
    template<class FUNC>
    static auto y_group(const std::vector<MKRegion>& regs, FUNC&& func) {
        Group<decltype(func(std::declval<MKRegion>()))> group;
        for(auto&& r : regs ) {
            group[r.y_i].push_back(func(r));
        }
        return group;
    }

    static auto x_group_points( const std::vector<MKRegion>& regs ) {
        return x_group(regs, [](const MKRegion& mk_reg){
            return cv::Point(mk_reg.x, mk_reg.y);
        });
    }
    static auto y_group_points( const std::vector<MKRegion>& regs ) {
        return y_group(regs, [](const MKRegion& mk_reg){
            return cv::Point(mk_reg.x, mk_reg.y);
        });
    }
};
}}}