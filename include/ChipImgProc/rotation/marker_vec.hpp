#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
#include <boost/math/constants/constants.hpp>
#include <map>
#include <vector>
#include <ChipImgProc/rotation/grid_point_infer.hpp>
namespace chipimgproc { namespace rotation {

template<class FLOAT>
struct MarkerVec : public GridPointInfer<FLOAT> {
    using Base = GridPointInfer<FLOAT>;
    using DirPointsGroup = typename Base::DirPointsGroup;
private:
    auto x_y_group( const std::vector<marker::detection::MKRegion>& set ) const {
        DirPointsGroup x_group, y_group;
        for(auto&& mk_r : set ) {
            x_group[mk_r.x_i].push_back(cv::Point(mk_r.x, mk_r.y));
            y_group[mk_r.y_i].push_back(cv::Point(mk_r.x, mk_r.y));
        }
        return nucleona::make_tuple(
            std::move(x_group), std::move(y_group)
        );
    }
    
public:
    auto operator()(
        const std::vector<
            marker::detection::MKRegion
        >&                                      mk_regions                              ,
        std::ostream&                           logger     = nucleona::stream::null_out
    ) const {
        auto[x_group, y_group] = x_y_group(mk_regions);
        return Base::operator()(x_group, y_group);
    }
};

}}