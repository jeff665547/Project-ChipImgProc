/**
 * @file marker_vec.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::rotation::MarkerVec
 * 
 */
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
/**
 * @brief This is the rotation angle estimator based on the grid of marker locations.
 *        Given a set of marker locations represented as chipimgproc::detection::MKRegion object,
 *        this algorithm firstly groups the marker locations into a set of nearly horizontal lines and vertical lines.
 *        Next, it calculates the mean of the inclination angles of nearly horizontal and vertical lines 
 *        against the standard basis vector (0,1) and (1,0) respectively by using
 *        the inherited template class chipimgproc::rotation::GridPointInfer.
 * 
 * @details The following shows an usage example:
 *   @snippet ChipImgProc/rotation/marker_vec_test.cpp usage
 * 
 * @tparam FLOAT the type of floating point type for rotation angle estimation
 */
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
    /**
     * @brief Given a set of marker regions, estimate the corresponding rotation angle
     * 
     * @param mk_regions    a set of marker regions. Please refer to MKRegion for details
     * @param logger        an output stream object for debug logging messages
     * @return auto         the rotation angle of the grid of marker regions.
     *                      The return type will be the template parameter FLOAT.
     */
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