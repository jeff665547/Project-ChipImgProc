/**
 * @file    ChipImgProc/aruco/utils.hpp
 * @author  Chia-Hua Chang, Alex Lee
 * @brief   ArUco marker utility functions
 */
#pragma once
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <cstdint>
#include <nlohmann/json.hpp>
namespace chipimgproc::aruco {
/**
 *  @brief A struct wrapper of utility functions
 */
struct Utils {
    /**
     *  @brief Compute one(1) numbers of a integer in binary form.
     *  @param  x   The input integer.
     *  @return The number of one in x's binary form.
     */
    static std::int32_t bit_count(std::uint64_t x) {
        x -= (x >> 1) & 0x5555555555555555;
        x  = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
        x  = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
        return (x * 0x0101010101010101) >> 56;
    }
    
    /**
     *  @brief Encode ArUco marker into integer.
     *  @param array ArUco marker specified in 1 and 0.
     *  @return A integer represent the ArUco marker.
     */
    static std::uint64_t encode(cv::Mat_<std::uint8_t> array) {
        std::uint64_t code = 0;
        for (auto offset = 0; offset != array.total(); ++offset)
            code |= static_cast<std::uint64_t>(array(offset)) << offset;
        return code;
    }
    /**
     *  @brief Decode integer into ArUco marker.
     *  @param code         The decoded integer.
     *  @param coding_bits  The ArUco marker's coding bits.
     *  @return ArUco marker decoding from integer code.
     */
    static cv::Mat_<std::uint8_t> decode(std::uint64_t code, std::int32_t coding_bits) {
        cv::Mat_<std::uint8_t> image(coding_bits, coding_bits);
        for (auto offset = image.total(); offset-- > 0; )
            image(offset) = static_cast<std::uint8_t>((code & (1ull << offset)) > 0);
        return image;
    }
    /**
     *  @brief  Extract ArUco markers id from raw ArUco marker dictionary json.
     *  @param  id_map The raw Aruco marker dictionary json.
     *  @return Ids vector.
     */
    static std::vector<std::int32_t> aruco_ids( const nlohmann::json& id_map) {
        std::vector<std::int32_t> res;
        res.reserve(id_map.size());
        for(auto it = id_map.begin(); it != id_map.end(); it ++ ) {
            res.push_back(std::stoi(it.key()));
        }
        return res;
    }
    /**
     *  @brief  Extract ArUco markers id and point from raw ArUco marker dictionary json.
     *  @param  id_map The raw Aruco marker dictionary json.
     *  @return id to point vector.
     */
    static std::vector<cv::Point> aruco_points( const nlohmann::json& id_map) {
        std::vector<cv::Point> res;

        std::int32_t max_id = 0;
        for(auto it = id_map.begin(); it != id_map.end(); it ++ ) {
            auto id = std::stoi(it.key());
            if(max_id < id) {
                max_id = id;
            }
        }
        res.resize(max_id + 1);
        for(auto it = id_map.begin(); it != id_map.end(); it ++ ) {
            auto id = std::stoi(it.key());
            auto arr = *it;
            res[id] = cv::Point(arr[0], arr[1]);
        }
        return res;
    }
    // static std::map<cv::Point, int, PointLess> aruco_point_to_ids(const nlohmann::json& id_map) {
    //     std::map<cv::Point, int, PointLess> res;
    //     for(auto it = id_map.begin(); it != id_map.end(); it ++ ) {
    //         auto id = std::stoi(it.key());
    //         auto arr = *it;
    //         cv::Point point(arr[0], arr[1]);
    //         res[point] = id;
    //     }
    //     return res;
    // }
    /**
     *  @brief Convert points from float to integer.
     *  @param p_vec A point container which element should be 
     *               a pair of a integer id and a float type point.
     *  @return A points container which element should be a integer id a float type point.
     */
    template<class P_VEC>
    static auto to_int_point( const P_VEC& p_vec ) {
        std::vector<std::tuple<std::int32_t, cv::Point>> res;
        for(auto& [id, p] : p_vec ) {
            res.emplace_back(id, cv::Point(
                std::round(p.x),
                std::round(p.y)
            ));
        }
        return res;
    }

    static void resize(
        cv::InputArray src
      , cv::OutputArray dst
      , double fx
      , double fy = 0
      , std::int32_t interplation = cv::INTER_AREA
    ) {
        if (fy == 0) fy = fx;
        double w0 = src.getMat().cols, h0 = src.getMat().rows;
        double w1 = std::round(w0 * fx), h1 = std::round(h0 * fy);
        double x0 = (w0 - 1) * 0.5, x1 = (w1 - 1) * 0.5;
        double y0 = (h0 - 1) * 0.5, y1 = (h1 - 1) * 0.5;
        cv::Matx23f M(fx, 0, x1 - x0 * fx, 0, fy, y1 - y0 * fy);
        cv::warpAffine(src, dst, M, cv::Size(w1, h1), interplation);
    }
};

}