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
    void set_dict(const std::string& path) {
        std::ifstream fin(path);
        nlohmann::json db;
        fin >> db;
        dictionary_ = aruco::Dictionary::from_json(db);
        dictionary_set_ = true;
    }
    void set_detector(
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
            res.emplace_back(
                x, y, marker.cols, marker.rows,
                
            );
            // res.push_back(
            //     cv::Rect(
            //         x, y, 
            //         marker.cols, marker.rows
            //     )
            // );
        }
        return res;
    }
private:
    aruco::Detector         detector_               ;

    bool                    dictionary_set_ {false} ;
    aruco::Dictionary       dictionary_             ;
};

}