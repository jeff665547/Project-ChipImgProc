#pragma once
#include <Nucleona/range.hpp>
#include <ChipImgProc/wrapper/transform_range.hpp>
namespace chipimgproc{ namespace wrapper{

template<class ARR>
struct IndexedRangeFunc {

    IndexedRangeFunc(ARR& mat)
    : inter_(mat)
    {}

    template<class IDX>
    decltype(auto) operator()( const IDX& id ) const{
        auto&& value = inter_.at(id);
        return value;
    }
private:
    ARR& inter_;
};
template<class ARR, class IDX_MAT>
using IndexedRangeBase = TransformRange<
    IDX_MAT, IndexedRangeFunc<ARR>
>;
// decltype(nucleona::range::transform(
//     std::declval<IDX_MAT&>(),
//     IndexedRangeFunc<ARR>(std::declval<ARR&>()),
//     std::declval<typename IDX_MAT::iterator::iterator_category>()
// ));

template<class ARR, class IDX_MAT>
struct IndexedRange : public IndexedRangeBase<ARR, IDX_MAT>{
    using Base = IndexedRangeBase<ARR, IDX_MAT>;
    IndexedRange(ARR& arr, IDX_MAT& idx_mat)
    : Base(nucleona::range::transform(
        idx_mat, 
        IndexedRangeFunc<ARR>(arr),
        typename Base::iterator_category()
    ))
    {}
};

}}