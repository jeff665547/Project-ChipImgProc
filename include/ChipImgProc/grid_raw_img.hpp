/**
 * @file grid_raw_img.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::GridRawImg
 * 
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <Nucleona/range.hpp>
#include <iostream>
namespace chipimgproc{
/**
 * @brief Type for storing rotated grid raw image.
 * 
 * @tparam GLID grid line storage type.
 */
template<class GLID = std::uint16_t>
struct GridRawImg {
    /**
     * @brief Create empty GridRawImg.
     * 
     */
    GridRawImg() = default;

    /**
     * @brief Constructor, initial GridRawImg with rotated image and grid lines.
     * 
     * @tparam MAT Deduced, matrix type, must be cv::Mat or cv::Mat&.
     * @tparam GLX Deduced, grid lines, 
     *   must be range type and the value can be convert to GLID.
     * @tparam GLY Deduced, grid lines, 
     *   must be range type and the value can be convert to GLID.
     * @param img Rotated raw image.
     * @param glx Grid line range.
     * @param gly Grid line range.
     */
    template<class MAT, class GLX, class GLY>
    GridRawImg(
        MAT&& img, 
        const GLX& glx,
        const GLY& gly
    )
    : img_  (FWD(img))
    , gl_x_ ()
    , gl_y_ ()
    {
        gl_x_.reserve(glx.size());
        gl_y_.reserve(gly.size());
        for(auto&& x : glx) {
            gl_x_.push_back(static_cast<GLID>(std::round(x)));
        }
        for(auto&& y : gly) {
            gl_y_.push_back(static_cast<GLID>(std::round(y)));
        }
    }
    
    /**
     * @brief Get internal image.
     * 
     * @return const cv::Mat& Reference to internal image.
     */
    const cv::Mat& mat() const {
        return img_;
    }
    
    /**
     * @brief Mutable version of chipimgproc::GridRawImg::mat() const.
     * 
     */
    cv::Mat& mat() {
        return img_;
    }
    
    /**
     * @brief Get X direction grid line.
     * 
     * @return const std::vector<GLID>& Reference to X direction grid line.
     */
    const std::vector<GLID>& gl_x() const  { return gl_x_; }
    
    /**
     * @brief Mutable version of chipimgproc::GridRawImg::gl_x() const
     * 
     * @details Be careful to use this version, once you use reference type to receive the return object.
     *   Andy modification to the reference may have side effects.
     */
    std::vector<GLID>&       gl_x()        { return gl_x_; }
    
    /**
     * @brief Get Y direction grid line.
     * 
     * @return const std::vector<GLID>& Reference to Y direction grid line.
     */
    const std::vector<GLID>& gl_y() const  { return gl_y_; }

    /**
     * @brief Mutable version of chipimgproc::GridRawImg::gl_y() const
     * 
     * @details Be careful to use this version, once you use reference type to receive the return object.
     *   Andy modification to the reference may have side effects.
     */
    std::vector<GLID>&       gl_y()        { return gl_y_; }

    /**
     * @brief Test if the object is empty.
     * 
     * @return true The object is empty.
     * @return false The object is assigned image.
     */
    bool        empty() const { return img_.empty(); }

    /**
     * @brief Get the row number of the image. It's pixel level, not cell level.
     * 
     * @return auto Deduced, same as cv::Mat::rows.
     */
    auto        rows()  const { return img_.rows; } 

    /**
     * @brief Get the col number of the image. It's pixel level, not cell level.
     * 
     * @return auto Deduced, same as cv::Mat::cols.
     */
    auto        cols()  const { return img_.cols; } 

    /**
     * @brief Get region of interested in cell level.
     * 
     * @param r The rectangle identify the region.
     * @return GridRawImg the region identified.
     */
    GridRawImg  get_roi(const cv::Rect& r) const {
        cv::Rect raw_rect(
            gl_x_.at(r.x), gl_y_.at(r.y),
            gl_x_.at(r.x + r.width ) - gl_x_.at(r.x),
            gl_y_.at(r.y + r.height) - gl_y_.at(r.y)
        );
        // std::cerr
        // << "r = "
        // << r.x << ", "
        // << r.y << ", "
        // << r.width  << ", "
        // << r.height << "\n";
        // std::cerr
        // << "raw_rect = "
        // << raw_rect.x << ", "
        // << raw_rect.y << ", "
        // << raw_rect.width << ", "
        // << raw_rect.height << "\n";
        auto img = img_(raw_rect); //
        // std::cerr << __FILE__ << ":" << __LINE__ << "\n";
        std::vector<GLID> gl_x(
            gl_x_.begin() + r.x, 
            gl_x_.begin() + r.x + r.width + 1
        );
        std::vector<GLID> gl_y(
            gl_y_.begin() + r.y, 
            gl_y_.begin() + r.y + r.height + 1
        );
        auto front = gl_x.front();
        for(auto& l : gl_x) {
            l -= front;
        }
        front = gl_y.front();
        for(auto& l : gl_y) {
            l -= front;
        }
        return GridRawImg(img, gl_x, gl_y);
    }
    /**
     * @brief Get the image only contain 
     *   the region from the first grid line to the last.
     * @details The grid rotated image may have some unused border 
     *   which has no probe location or just not the FOV defined grid region.
     *   This function is used to trim such part of image and leave only 
     *   the grid line located part of the image.
     * @return GridRawImg The image after border trimmed. 
     */
    GridRawImg clean_border() const {
        cv::Rect roi(
            0, 0, 
            gl_x_.size() - 1, gl_y_.size() - 1
        );
        return get_roi(roi);
    }

    /**
     * @brief Duplicate everything in the image. Doing deep copy.
     * 
     * @return GridRawImg Same data of current image.
     */
    GridRawImg clone() const {
        return GridRawImg(
            img_.clone(),
            gl_x_, gl_y_
        );
    }

private:
    cv::Mat img_;
    std::vector<GLID> gl_x_;
    std::vector<GLID> gl_y_;
};



}
