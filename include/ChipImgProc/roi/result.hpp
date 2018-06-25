#pragma once
namespace chipimgproc{ namespace roi{

struct Result {
    enum QC{
        fail, pass, unknown
    };

    QC qc;
};

}}