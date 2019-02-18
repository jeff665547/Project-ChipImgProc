#pragma once
#include <ChipImgProc/wrapper/transform_range.hpp>
namespace chipimgproc{ namespace wrapper{

template<class MAT, class ACC_FUNC>
struct AccBindImpl {
    AccBindImpl(MAT&& mat, ACC_FUNC&& acc_func)
    : inter_        (FWD(mat))
    , acc_func_     (FWD(acc_func))
    {}

    decltype(auto) at(int row, int col) {
        return inter_.at(row, col, acc_func_);
    }
    decltype(auto) at(int row, int col) const {
        return inter_.at(row, col, acc_func_);
    }
    decltype(auto) operator()(int row, int col) {
        return inter_(row, col, acc_func_);
    }
    decltype(auto) operator()(int row, int col) const {
        return inter_(row, col, acc_func_);
    }
    decltype(auto) dump() {
        return inter_.dump(acc_func_);
    }
    decltype(auto) dump() const {
        return inter_.dump(acc_func_);
    }
protected:
    MAT         inter_      ;
    ACC_FUNC    acc_func_   ;
};


template<class MAT, class ACC_FUNC>
struct AccBind 
: public AccBindImpl<MAT, ACC_FUNC>
, public ranges::transform_view<
    ranges::view::all_t<MAT>, ACC_FUNC>
{
    AccBind(MAT&& mat, ACC_FUNC acc_func)
    : AccBindImpl   <MAT , ACC_FUNC>(FWD(mat), nucleona::copy(acc_func))
    , ranges::transform_view<
        ranges::view::all_t<MAT>, 
        ACC_FUNC
    >(
        ranges::view::all(static_cast<MAT&&>(mat)),
        std::move(acc_func)
    )
    {}
    using AccBindImpl<MAT, ACC_FUNC>::operator();
};

template<class MAT, class ACC_FUNC>
decltype(auto) bind_acc(MAT&& mat, ACC_FUNC acc_func) {
    return AccBind<MAT, ACC_FUNC>(FWD(mat), std::move(acc_func));
}

}}