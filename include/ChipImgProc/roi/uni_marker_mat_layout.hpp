#pragma once
#include <ChipImgProc/marker_layout.hpp>
#include <ChipImgProc/roi/result.hpp>
#include <ChipImgProc/stat/mats.hpp>
#include <ChipImgProc/tiled_mat.hpp>
#include <map>
#include <ChipImgProc/utils.h>
namespace chipimgproc{ namespace roi{
struct UniMarkerMatLayout {
    cv::Mat binarize(const cv::Mat_<float>& m) {
        auto trimmed_mean = trim_outlier(m.clone(), 0, 0.02); // TODO: smarter way
        cv::Mat_<std::uint8_t> bin;
        cv::normalize(trimmed_mean, bin, 0, 255, cv::NORM_MINMAX, bin.depth());
        return bin;
    }
    auto check_marker_position(
        const cv::Mat_<std::uint16_t>& tgt_mk_pos_x,
        const cv::Mat_<std::uint16_t>& tgt_mk_pos_y,
        const MarkerLayout& mk_layout,
        int threshold = 2,
        std::ostream& logger = nucleona::stream::null_out
    ) {
        cv::Point op(tgt_mk_pos_x(0, 0), tgt_mk_pos_y(0, 0));
        std::map<cv::Point, int, PointLess> vote_tab(point_less);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        for( int r = 0; r < tgt_mk_pos_x.rows; r ++ ) {
            for ( int c = 0; c < tgt_mk_pos_x.cols; c ++ ) {
                std::uint16_t x = tgt_mk_pos_x(r, c);
                std::uint16_t y = tgt_mk_pos_y(r, c);
                cv::Point p(x, y);
                p.x -= (mk_layout.mk_invl_x_cl * c);
                p.y -= (mk_layout.mk_invl_y_cl * r);
                vote_tab[p] ++;
            }
        }
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        auto max_vote = vote_tab.begin();
        for ( auto itr = vote_tab.begin(); itr != vote_tab.end(); itr ++ ) {
            logger << itr->first << " : " << itr->second << std::endl;
            if( itr->second > max_vote->second ) {
                max_vote = itr;
            }
        }
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        struct {
            bool qc_pass;
            cv::Point org_point;
        } res {
            max_vote->second > threshold,
            max_vote->first - mk_layout.get_marker_des(0, 0).get_pos()
        };
        return res;
    }
    template<class GLID>
    bool operator()( 
        const MarkerLayout&        mk_layout,
        stat::Mats&                raw_smats,
        TiledMat<GLID>&            tiled_src,
        std::ostream&              out       = nucleona::stream::null_out,
        const std::function<
            void(const cv::Mat&)
        >&                         v_bin     = nullptr,
        const std::function<
            void(const cv::Mat&, int, int)
        >&                         v_score   = nullptr,
        const std::function<
            void(const cv::Mat&)
        >&                         v_result  = nullptr
    ) {
        auto tgt = binarize(raw_smats.mean); // TODO:
        if(v_bin) v_bin(tgt);
        auto sp_h = tgt.rows / mk_layout.mk_map.rows;
        auto sp_w = tgt.cols / mk_layout.mk_map.cols;
        out << "sub width : " << sp_w << std::endl;
        out << "sub height: " << sp_h << std::endl;
        cv::Mat_<std::uint16_t> found_mk_pos_x(
            mk_layout.mk_map.rows,
            mk_layout.mk_map.cols
        );
        cv::Mat_<std::uint16_t> found_mk_pos_y(
            mk_layout.mk_map.rows,
            mk_layout.mk_map.cols
        );
        for( int r = 0; r < mk_layout.mk_map.rows; r ++ ) {
            for( int c = 0; c < mk_layout.mk_map.cols; c ++ ) {
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                std::int16_t mk_des_idx = mk_layout.mk_map(c, r);
                auto& mk_des = mk_layout.mks.at(mk_des_idx);
                auto& candi_mks = mk_des.candi_mks;
                cv::Rect match_region(
                    c * sp_w,
                    r * sp_h,
                    sp_w,
                    sp_h
                );
                out << "sub region (r,c): (" << r << "," << c << "): " << match_region << std::endl;
                cv::Mat sub_tgt = tgt(match_region); 
                cv::Mat_<float> sub_score;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                for(auto& mk : candi_mks) {
                    cv::Mat_<float> sub_candi_score(
                        sub_tgt.cols - mk.cols + 1,
                        sub_tgt.rows - mk.rows + 1
                    );
                    cv::imwrite("debug_sub_tgt.tiff", sub_tgt);
                    cv::imwrite("debug_mk.tiff", mk); // TODO: get bad marker here
                    cv::matchTemplate(sub_tgt, mk, sub_candi_score, CV_TM_CCORR_NORMED);
                    if(sub_score.empty()) 
                        sub_score = sub_candi_score;
                    else 
                        sub_score += sub_candi_score;
                }
                if(v_score) {
                    cv::Mat tmp = sub_score + 1;
                    cv::Mat tmp2;
                    tmp.convertTo(tmp2, CV_16UC1, 65535, 0);
                    v_score(viewable(tmp2), r, c);
                }
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                double min, max;
                cv::Point min_loc, max_loc;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                cv::minMaxLoc(sub_score, &min, &max, &min_loc, &max_loc);
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                max_loc.x += match_region.x;
                max_loc.y += match_region.y;
                found_mk_pos_x(r, c) = max_loc.x;
                found_mk_pos_y(r, c) = max_loc.y;
                std::cout << __FILE__ << ":" << __LINE__ << std::endl;
            }
        }
        out << "match mk loc x: " << std::endl;
        out << found_mk_pos_x << std::endl;
        out << "match mk loc y: " << std::endl;
        out << found_mk_pos_y << std::endl;
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        auto check_res = check_marker_position(
            found_mk_pos_x, 
            found_mk_pos_y,
            mk_layout,
            2, 
            out
        );
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        cv::Rect r(
            check_res.org_point.x,
            check_res.org_point.y,
            check_res.org_point.x + 
                mk_layout.mk_invl_x_cl * mk_layout.mk_map.cols,
            check_res.org_point.y + 
                mk_layout.mk_invl_y_cl * mk_layout.mk_map.rows
        );
        out << "cell roi: " << r << std::endl;

        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        tiled_src.roi(r);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        raw_smats.roi(r);
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        if(v_result) {
            auto tmp = tiled_src.get_roi_image();
            v_result(viewable(tmp));
        }
        std::cout << __FILE__ << ":" << __LINE__ << std::endl;
        
        return check_res.qc_pass;
        
    }
};


}}