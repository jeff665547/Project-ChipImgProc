#pragma once
#include <vector>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/obj_mat.hpp>
namespace chipimgproc::marker {

struct CellLayout {
    struct Rect : public cv::Rect {
        int x_i, y_i;
    };
    struct FOV : public Rect{
        ObjMat<cv::Point> ol_mks;
    };
    void reset(
        int fov_rows, int fov_cols,
        int fov_w,    int fov_h,
        int fov_w_d,  int fov_h_d,
        int mk_rows,  int mk_cols,
        int mk_x_i,   int mk_y_i,
        int mk_w,     int mk_h,
        int mk_w_d,   int mk_h_d
    ) {
        ObjMat<std::vector<cv::Point>> fov_oli;
        fovs_.resize(fov_rows, fov_cols);
        markers_.resize(mk_rows, mk_cols);
        fov_oli.resize(fov_rows, fov_cols);
        for(int i = 0; i < mk_rows; i ++) {
            for(int j = 0; j < mk_cols; j ++ ) {
                auto& r = markers_(i, j);
                r.x = j * mk_w_d;
                r.y = i * mk_h_d;
                r.width = mk_w;
                r.height = mk_h;
                r.x_i = j;
                r.y_i = i;
            }
        }
        for(int i = 0; i < fov_rows; i ++) {
            for(int j = 0; j < fov_cols; j ++ ) {
                auto& r = fovs_(i, j);
                r.x = j * fov_w_d;
                r.y = i * fov_h_d;
                r.width = fov_w;
                r.height = fov_h;
                r.x_i = j;
                r.y_i = i;
                for(auto& mk : markers_.values()) {
                    auto ovp = mk & r;
                    if(ovp.width > 0 && ovp.height > 0) {
                        fov_oli(i, j).emplace_back(mk.x_i, mk.y_i);
                    }
                }
            }
        }
        for(int i = 0; i < fov_rows; i ++) {
            for(int j = 0; j < fov_cols; j ++ ) {
                auto& fov = fovs_(i, j);
                auto& mk_ids = fov_oli(i, j);
                int x = 0, y = 0;
                int last_mk_id_x = -1;
                for(auto& mk_id : mk_ids) {
                    if(mk_id.x <= last_mk_id_x) {
                        x = 0;
                        y++;
                    }
                    // f(x): 0, 1, 2, 0, 1, 2, 0, 1, 2, 
                    // f(y): 0, 0, 0, 1, 1, 1, 2, 2, 2, 3,
                    x ++;
                    last_mk_id_x = mk_id.x;
                }
                fov.ol_mks.resize(y + 1, x);
                x = 0; y = 0; last_mk_id_x = -1;
                for(auto& mk_id : mk_ids) {
                    if(mk_id.x <= last_mk_id_x) {
                        x = 0;
                        y++;
                    }
                    fov.ol_mks(y, x++) = mk_id;
                    last_mk_id_x = mk_id.x;
                }
            }
        }
    }
    auto& fov(int r, int c) {
        return fovs_(r,c);
    }
    const auto& fov(int r, int c) const {
        return fovs_(r,c);
    }
    auto& marker(int r, int c) {
        return markers_(r,c);
    }
    const auto& marker(int r, int c) const {
        return markers_(r,c);
    }
    auto mk_id_fov_to_chip(int fov_id_x, int fov_id_y) const {
        return [this, fov_id_x, fov_id_y](int x_i, int y_i) {
            auto& fov = fovs_(fov_id_y, fov_id_x);
            return fov.ol_mks(y_i, x_i);
        };
    }
private:
    ObjMat<FOV>         fovs_       ;
    ObjMat<Rect>        markers_    ;

};

}