#pragma once
#include <Nucleona/range.hpp>
// #include <ChipImgProc/wrapper/transform_range.hpp>
#include <range/v3/view/transform.hpp>
namespace chipimgproc{ namespace wrapper{

template<class ARR>
struct IndexedRangeFunc {

    IndexedRangeFunc() = default;
    IndexedRangeFunc(ARR& mat)
    : inter_(&mat)
    {}

    template<class IDX>
    decltype(auto) operator()( const IDX& id ) const{
        auto&& value = inter_->at(id);
        return value;
    }
private:
    ARR* inter_ {nullptr};
};

template<class Arr, class IdxMat>
using IndexedRangeBase = ranges::transform_view<
    ranges::view::all_t<IdxMat&>, IndexedRangeFunc<Arr>
>;

template<class Arr, class IdxMat>
struct IndexedRange : IndexedRangeBase<Arr, IdxMat> {
    using Base = IndexedRangeBase<Arr, IdxMat>;
    IndexedRange() = default;
    IndexedRange(Arr& arr, IdxMat& idx_mat)
    : Base { 
        ranges::view::all(static_cast<IdxMat&>(idx_mat)), 
        IndexedRangeFunc<Arr>(arr) 
    }
    {}
};

}}