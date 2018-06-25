#pragma once
#include <ChipImgProc/utils.h>
namespace chipimgproc{ namespace marker{
struct Des {
friend struct Layout;
    const cv::Point& get_pos() const {
        return pos_;
    }
    std::vector<cv::Mat_<std::uint8_t>> candi_mks;
private:
    cv::Point pos_;
};
struct Layout {
    enum PatternNum {
        single, multi
    };
    enum DistForm {
        uni_mat, random
    };
    const Des& get_marker_des(int r, int c) const {
        if( dist_form != uni_mat) {
            throw std::runtime_error("get_marker_des(r,c) only support uni_mat form");
        }
        return mks.at(mk_map(r, c));
    }
    Des& get_marker_des(int r, int c) {
        return const_cast<Des&>(
            static_cast<const decltype(this)>(this)->get_marker_des(r,c)
        );
    }
    void set_uni_mat_dist(
        int rows, int cols, 
        const cv::Point& min_p, 
        std::uint32_t invl_x_cl, std::uint32_t invl_y_cl
    ) {
        dist_form = uni_mat;
        mk_map = decltype(mk_map)(rows, cols);
        mks.resize(rows * cols);
        mks.at(0).pos_ = min_p;
        for(int r = 0; r < rows; r ++ ) {
            for( int c = 0; c < cols; c ++ ) {
                decltype(mk_map)::value_type idx = r * cols + c;
                mk_map(r, c) = idx;
                mks.at(idx).pos_.x = min_p.x + invl_x_cl * c;
                mks.at(idx).pos_.y = min_p.y + invl_y_cl * r;
            }
        }
        mk_invl_x_cl = invl_x_cl;
        mk_invl_y_cl = invl_y_cl;
    }
    void set_single_mk_pat( const std::vector<cv::Mat_<std::uint8_t>>& candi_pats ) {
        pat_num = single;
        for( auto& mk : mks ) {
            mk.candi_mks = candi_pats;
        }
    } 
    void set_mk_pat( const cv::Point& p, const std::vector<cv::Mat_<std::uint8_t>>& candi_pats) {
        if( pat_num != multi) {
            throw std::runtime_error("set_mk_pat(p, candi_pats) is used in multi pattern type");
        }
        // if( dist_form != random ) {
        //     throw std::runtime_error("set_mk_pat(p, candi_pats) is used in random distribution type");
        // }
        if( dist_form == random ) {
            Des mk;
            mk.candi_mks = candi_pats;
            mk.pos_ = p;
            mks.push_back(mk);
        } else if( dist_form == uni_mat) {
            get_marker_des(p.y, p.x).candi_mks = candi_pats;
        }
    }
    void info(std::ostream& out) {
    
    }
    int get_marker_width() const {
        return mks.at(0).candi_mks.at(0).cols;
    }
    int get_marker_height() const {
        return mks.at(0).candi_mks.at(0).rows;
    }
    // void set_mk_pat_for_uni_mat_dist( const cv::Point& ) {

    // }

    PatternNum              pat_num        { multi  } ;
    DistForm                dist_form      { random } ;
    cv::Mat_<std::int16_t>  mk_map                    ; // used in uni_mat
    std::vector<Des>        mks                       ; // used in all type
    std::uint32_t           mk_invl_x_cl              ; // used in uni_mat
    std::uint32_t           mk_invl_y_cl              ; // used in uni_mat
};


}}