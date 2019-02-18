#pragma once
#include <Nucleona/range.hpp>
namespace chipimgproc{ namespace wrapper{

template<class RNG, class FUNC>
using TransformRange = ranges::transform_view<RNG, FUNC> ;

}}