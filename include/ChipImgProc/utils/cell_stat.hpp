#pragma once

namespace chipimgproc {
struct StatIdx {
    enum {
        means   = 0,
        stddev  = 1,
        cv      = 2,
        num     = 3,
    };
};
struct CellStat
{
    float mean;
    float stddev;
    float cv;
    std::uint32_t num;
};

}