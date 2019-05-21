#pragma once
#include <vector>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/const.h>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <ChipImgProc/aruco.hpp>
namespace chipimgproc::marker::detection {

struct ArucoRegMat {
    void set_dict(const nlohmann::json& db, const std::string& key) {
        auto& dict_json = db[key];
        mk_index_map_.resize(dict_json["bitmap_list"].size());
        auto dict = aruco::Dictionary::from_json(dict_json);
        set_dict_(std::move(dict));
    }
    void set_dict(const std::string& path, const std::string& key) {
        std::ifstream fin(path);
        nlohmann::json db;
        fin >> db;
        set_dict(db, key);
    }
    void set_detector_ext(
        const std::int32_t&               pyramid_level
      , const std::int32_t&               border_bits               // bits
      , const std::int32_t&               fringe_bits               // bits
      , const double&                     a_bit_width               // pixels
      , const double&                     margin_size               // pixels 
      , const std::string&                frame_template_path
      , const std::string&                frame_mask_path
      , const std::int32_t&               nms_count
      , const double&                     nms_radius                // pixels
      , const std::int32_t&               cell_size                 // pixels
      , const std::vector<std::int32_t>&  ids
      , const std::uint16_t&              mk_rows
      , const std::uint16_t&              mk_cols
      , std::ostream&                     logger            = nucleona::stream::null_out
    ) {
        if(!dictionary_set_)
            throw std::runtime_error("dictionary not yet set");
        auto frame_template = cv::imread(frame_template_path, cv::IMREAD_GRAYSCALE);
        auto frame_mask     = cv::imread(frame_mask_path, cv::IMREAD_GRAYSCALE);
        detector_.reset(
            dictionary_, pyramid_level, 
            border_bits, fringe_bits,
            a_bit_width,
            margin_size, 
            frame_template, frame_mask,
            nms_count, nms_radius,
            cell_size,
            ids,
            logger
        );
        int x_i(0), y_i(0);
        for(auto&& id : ids) {
            if(x_i == 6) {
                y_i ++;
                x_i = 0;
            }
            mk_index_map_.at(id) = cv::Point(x_i, y_i);
            x_i ++;
        }
    }
    template<class T>
    std::vector<MKRegion> operator()(
        const cv::Mat_<T>&          src, 
        const Layout&               mk_layout, 
        const MatUnit&              unit,
        std::ostream&               out        = nucleona::stream::null_out,
        const ViewerCallback&       v_bin      = nullptr,
        const ViewerCallback&       v_search   = nullptr,
        const ViewerCallback&       v_marker   = nullptr
    ) const {
        std::vector<MKRegion> res;
        auto& mk_des = mk_layout.get_single_pat_marker_des();
        auto& marker = mk_des.get_std_mk(unit);
        auto pts = detector_.detect_markers(src, out);
        res.reserve(pts.size());
        for(auto [i, p] : pts) {
            auto x = p.x - (marker.cols / 2);
            auto y = p.y - (marker.rows / 2);
            auto& mk_idx = mk_index_map_.at(i);
            MKRegion mk_r;
            mk_r.x = x;
            mk_r.y = y;
            mk_r.width = marker.cols;
            mk_r.height = marker.rows;
            mk_r.x_i = mk_idx.x;
            mk_r.y_i = mk_idx.y;
            mk_r.score = 0;
            res.push_back(mk_r);
        }
        return res;
    }
private:
    void set_dict_(aruco::Dictionary&& dict) {
        dictionary_ = std::move(dict);
        dictionary_set_ = true;
    }
    aruco::Detector         detector_               ;

    bool                    dictionary_set_ {false} ;
    aruco::Dictionary       dictionary_             ;

    std::vector<cv::Point>  mk_index_map_           ;
};

}