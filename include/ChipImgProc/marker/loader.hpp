#pragma once
#include <cstdint>
#include <ChipImgProc/utils.h>
#include <string>
#include <istream>
#include <Nucleona/algo/split.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <Nucleona/tuple.hpp>
#include "txt_to_img.hpp"
#include <fstream>
namespace chipimgproc{ namespace marker{

struct Loader {
    static auto from_txt(
        std::istream& is, 
        std::ostream& logger = nucleona::stream::null_out
    ) {
        std::string line;
        int rows = 0;
        int cols = 0;
        std::vector<std::uint8_t> mat_d;
        std::vector<std::uint8_t> mat_m;
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
                // process template
                switch(str.at(0)) {
                    case 'X':
                        mat_d.push_back(255);
                        break;
                    case '.': case 'M':
                        mat_d.push_back(0);
                        break;
                    case 'O':
                        mat_d.push_back(127);
                        break;
                    default:
                        throw std::runtime_error(
                            "pattern file format error:"
                            " the symbol only allow X or . charactor"
                        );
                }
                // process mask
                switch(str.at(0)) {
                    case 'M':
                        mat_m.push_back(0);
                        break;
                    default:
                        mat_m.push_back(255);
                        break;
                }
            }
            rows ++;
        }
        cv::Mat_<std::uint8_t> res(
            rows, cols, mat_d.data()
        );
        cv::Mat_<std::uint8_t> mask(
            rows, cols, mat_m.data()
        );
        logger << "load marker: " << std::endl;
        logger << res << std::endl;
        logger << "load mask: " << std::endl;
        logger << mask << std::endl;
        return nucleona::make_tuple(
            res.clone(),
            mask.clone()
        );
    }
    static auto from_file_to_img(
        const std::string& path,
        double cell_r,
        double cell_c,
        double sp,
        double scale = 1.0,
        double usamp = 8.0
    ) {
        std::ifstream marker_in(path);
        auto [templ, mask] = Loader::from_txt(marker_in);
        return txt_to_img(
            templ, 
            mask,
            cell_r, cell_c,
            sp, scale, usamp
        );
    }
};

}
}