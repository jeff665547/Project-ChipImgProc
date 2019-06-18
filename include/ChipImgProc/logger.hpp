#pragma once
#ifdef CHIPIMGPROC_ENABLE_LOG
#   include "logger/impl.hpp"
#else
#   include "logger/nolog_impl.hpp"
#endif