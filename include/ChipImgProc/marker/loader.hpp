#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <string>
#include <istream>
#include <nucleona/algo/split.hpp>
#include <nucleona/stream/null_buffer.hpp>
namespace chipimgproc{ namespace marker{

struct Loader {
    static cv::Mat_<std::uint8_t> from_txt(
        std::istream& is, 
        std::ostream& logger = nucleona::stream::null_out
    ) {
        std::string line;
        int rows = 0;
        int cols = 0;
        std::vector<std::uint8_t> mat_d;
        while(std::getline(is, line)) {
            auto entry = nucleona::algo::split(line, " ");
            if( cols == 0 ) {
                cols = entry.size();
            } else if ( cols != entry.size() ){
                throw std::runtime_error(
                    "pattern file format error: column not all equal"
                );
            }
            for( auto& str : entry) {
                if( str.size() != 1 ) {
                    throw std::runtime_error(
                        "pattern file format error:"
                        " the symobol should only one character"
                    );
                }
                switch(str.at(0)) {
                    case 'X':
                        mat_d.push_back(255);
                        break;
                    case '.':
                        mat_d.push_back(0);
                        break;
                    default:
                        throw std::runtime_error(
                            "pattern file format error:"
                            " the symbol only allow X or . charactor"
                        );
                }
            }
            rows ++;
        }
        cv::Mat_<std::uint8_t> res(
            rows, cols, mat_d.data()
        );
        logger << "load marker: " << std::endl;
        logger << res << std::endl;
        return res.clone();
    }
};

}
}