#pragma once
#include <Nucleona/range.hpp>
namespace chipimgproc{ namespace wrapper{

template<class RNG, class FUNC>
using TransformRange = std::decay_t<decltype(nucleona::range::transform(
    std::declval<RNG&>(), 
    std::declval<FUNC>(), 
    std::declval<typename std::decay_t<RNG>::iterator::iterator_category>()
))>;

}}