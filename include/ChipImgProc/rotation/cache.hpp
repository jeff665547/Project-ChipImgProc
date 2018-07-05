#pragma once
#include <cmath>
#include <random>
#include <ChipImgProc/utils.h>
#include <fstream>
#include <boost/filesystem.hpp>

namespace chipimgproc{ namespace rotation{
template<class FLOAT>
struct Cache {

    Cache()
    : enable_(false)
    {
        get_cache_ = [](const std::string& path) {
            return path + "_theta.txt";
        };
    }
    void set_img_id( const std::string& id) {
        if( id.empty() || id == "" ) {
            enable_ = false;
        } else {
            img_path_ = id;
            enable_ = true;
        }
    }
    void save_cache(FLOAT theta) {
        if( enable_ ) {
            auto cache_path = get_cache_(img_path_);
            std::ofstream fout(cache_path);
            fout << theta;
        }
    }
    bool has_cache() const {
        if( enable_ ) {
            return boost::filesystem::exists(get_cache_path());
        } else {
            return false;
        }
    }
    FLOAT get_cache() const {
        if( enable_ ) {
            auto cache_path = get_cache_path();
            std::ifstream fin(cache_path);
            FLOAT value;
            fin >> value;
            return value;
        } else {
            throw std::runtime_error("the rotate cache disable");
        }
    }
private:
    std::string get_cache_path() const {
        return get_cache_(img_path_);
    }
    std::function<std::string(const std::string&)> get_cache_;
    std::string img_path_;
    bool enable_;
};

}}