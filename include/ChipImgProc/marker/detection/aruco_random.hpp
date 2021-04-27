#pragma once
#include "random_based.hpp"
#include <ChipImgProc/aruco.hpp>

namespace chipimgproc::marker::detection {

struct MakeArucoRandom;
struct ArucoRandom : public RandomBased<ArucoRandom> {
    using Base = RandomBased<ArucoRandom>;
friend MakeArucoRandom;
protected:
    ArucoRandom(
        const aruco::ConstDictionaryPtr dict,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const double&                   theor_max_val,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius,
        const double&                   ext_width
    )
    : RandomBased(
        templ, mask, pyramid_level, theor_max_val,
        nms_count, nms_radius
    )
    , dict_         (dict)
    , ext_width_    (0)
    {
        assert(aruco_width >= dict_->coding_bits() * ext_width);

        auto n = dict_->coding_bits();
        auto a = aruco_width / n;
        auto b = 0.5 * (n - 1);
        auto s0 = templ.rows;
        auto s1 = templ.cols;
        anchors_.clear();
        for(std::size_t i = 0; i < n; i ++) {
            auto y = a * (i - b) + ((s0 - 1) * 0.5);
            for(std::size_t j = 0; j < n; j ++) {
                auto x = a * (j - b) + ((s1 - 1) * 0.5);
                anchors_.emplace_back(x, y);
            }
        }
        anchors_.emplace_back(
            (s1 - 1) * 0.5, 
            (s0 - 1) * 0.5
        );
        ext_width_ = std::round(a * ext_width);
    }
    std::uint64_t to_binary_(
        cv::Mat_<std::uint8_t>          patch, 
        const std::vector<cv::Vec2f>&   anchors,
        const int                       thres = 3
    ) const {
        auto coding_bits = this->dict_->coding_bits();
        auto num_anchors = coding_bits * coding_bits;
        cv::Rect roi(0, 0, ext_width_, ext_width_);
        cv::Mat_<std::uint8_t> tmp(roi.size() * coding_bits);
        for (auto i = 0; i != num_anchors; ++i) {
            roi.x = roi.width  * (i % coding_bits);
            roi.y = roi.height * (i / coding_bits);
            cv::getRectSubPix(patch, roi.size(), anchors[i], tmp(roi));
        }
        cv::Mat_<std::uint8_t> bw;
        cv::threshold(tmp, bw, 0, 1, cv::THRESH_BINARY | cv::THRESH_OTSU);
        std::uint64_t binary = 0ull;
        for (auto i = 0; i != num_anchors; ++i) {
            roi.x = roi.width  * (i % coding_bits);
            roi.y = roi.height * (i / coding_bits);
            std::uint64_t bit = cv::norm(bw(roi), cv::NORM_L1) > thres;
            binary |= bit << i;
        }
        return binary;
    }


public:

    RndMkId identify(
        const cv::Mat&                  patch,
        const std::vector<cv::Vec2f>&   anchors,
        const std::vector<int>&         active_ids,
        const int                       thres = 3
    ) const {
        // cv::imwrite("debug.png", patch);
        const auto& w           = ext_width_;
        const auto  coding_bits = dict_->coding_bits();
        const auto  num_anchors = coding_bits * coding_bits;
        const auto  tmp_wh      = coding_bits * w;
        cv::Mat_<std::uint8_t> tmp(tmp_wh, tmp_wh);

        if(num_anchors > anchors.size()) {
            throw std::runtime_error(
                "BUG: The anchors number not match the assumption of \"identify\" function"
            );
        }
        auto binary = to_binary_(patch, anchors, thres);
        auto [id, distance] = dict_->identify(binary, active_ids);
        return {id, distance};
    }

    auto operator()(
        cv::Mat                  input, 
        const std::vector<int>&  active_ids,
        const int                thres = 3
    ) const {
        return Base::operator()(input, active_ids, thres);
    }
private:
    aruco::ConstDictionaryPtr   dict_         ;
    int                         ext_width_    ;
};

struct MakeArucoRandom {
    ArucoRandom operator()(
        aruco::ConstDictionaryPtr       dict,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius,
        const double&                   ext_width
    ) const {
        double theor_max_val(16383);
        return ArucoRandom(
            dict, 
            templ,
            mask,
            aruco_width,
            pyramid_level,
            theor_max_val,
            nms_count,
            nms_radius,
            ext_width
        );
    }
    ArucoRandom operator()(
        aruco::Dictionary&&             dict,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const double&                   theor_max_val,
        const std::int32_t&             nms_count,
        const std::int32_t&             nms_radius,
        const double&                   ext_width
    ) const {
        aruco::ConstDictionaryPtr ptr(new aruco::Dictionary(std::move(dict)));
        return ArucoRandom(
            ptr,
            templ,
            mask,
            aruco_width,
            pyramid_level,
            theor_max_val,
            nms_count,
            nms_radius,
            ext_width
        );
    }
    ArucoRandom operator()(
        const std::string&              db_path,
        const std::string&              db_key,
        const cv::Mat_<std::uint8_t>&   templ,
        const cv::Mat_<std::uint8_t>&   mask,
        const double&                   aruco_width,
        const std::int32_t&             pyramid_level,
        const double&                   theor_max_val,
        const std::int32_t&             nms_count,
        const double&                   nms_radius,
        const double&                   ext_width
    ) const {
        return operator()(
            aruco::Dictionary::from_db_and_key(db_path, db_key),
            templ,
            mask,
            aruco_width,
            pyramid_level,
            theor_max_val,
            nms_count,
            nms_radius,
            ext_width
        );
    }
};
constexpr MakeArucoRandom make_aruco_random;

}
