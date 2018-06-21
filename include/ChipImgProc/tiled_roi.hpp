/**
 * @file    ChipImgProc/r_o_i_detection.hpp
 * @author  Alex Lee
 * @author  Chia-Hua Chang 
 * @brief   The Regioin of Interest detection algorithm implementation.
 * @details This module do following things : 
 * 1. Extract the exact intensities region of the intensities grid.
 * 2. Correct the coordinate system of intensities grid.
 * 3. Write back to the intensities grid.
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/tiled_mat.hpp>
// #include <ChipImgProc/chip_mark_layout.hpp>
// #include <ChipImgProc/coordinate_system_normalization.hpp>
// #include <ChipImgProc/r_o_i_qc.hpp>
namespace chipimgproc{

/**
 * @brief   The Regioin of Interest detection algorithm implementation.
 * @details The detail information can see here @ref improc_r_o_i_detection
 */
struct ROIDetection
{
  protected:
    /// @private 
    template<class T>
    static void vec_roi( 
          std::vector<T>& vec
        , std::size_t col_nums
        , const cv::Rect& roi
    )
    {
        std::vector<T> new_tiles;
        new_tiles.resize( roi.width * roi.height );
        auto&& rmv = cpt::view::make_row_major_view( vec, col_nums, vec.size() / col_nums );
        auto&& new_rmv = cpt::view::make_row_major_view( new_tiles, roi.width, roi.height );
        for ( auto i = roi.y; i < roi.y + roi.height; i ++ )
        {
            for ( auto j = roi.x; j < roi.x + roi.width; j ++ )
            {
                auto& t = rmv( i, j );
                new_rmv(i - roi.y, j - roi.x) = t;
            }
        }
        vec.swap( new_tiles );
    }
    /// @private 
    static auto get_layout_score(
          const cv::Mat_<float>& mean
        , const uint32_t& x_marker_num
        , const uint32_t& y_marker_num
        , const uint32_t& marker_x_interval
        , const uint32_t& marker_y_interval
        , const std::vector<cv::Mat_<uint8_t>>& markers
        , bool& roi_qc_fail
        , const std::string& img_prefix
        , const uint16_t& v_mean_trimmed
        , const uint16_t& v_mean_binarized
        , const uint16_t& v_mean_score
        , const uint16_t& v_layout_score
        , const bool& verbose = true
    )
    {
        auto trimmed_mean = trim_outlier ( mean.clone(), 0, 0.02 );
        // remove outlier
        cv_imshow(
              img_prefix + "_mean_trimmed.tif"
            , [&trimmed_mean]
            {
                cv::Mat tmp = trimmed_mean.clone();
                cv::normalize(tmp, tmp, 0, 65535, cv::NORM_MINMAX, CV_16U);
                return tmp;
            }
            , verbose
        );
        cpt::msg << VDUMP ( x_marker_num ) << std::endl;
        cpt::msg << VDUMP ( y_marker_num ) << std::endl;

        cv::Mat_<uint8_t> bw;
        cv::normalize(trimmed_mean, bw, 0, 255.0, cv::NORM_MINMAX, bw.depth());
        cv_imshow(
              img_prefix + "_mean_binarized.tif"
            , [&bw]
            {
                auto tmp = bw.clone();
                return tmp;
            }
            , verbose
        );
        // binarization

        cv::Mat_<float> scores;
        for ( auto&& marker : markers )
        {
            std::cout << marker << std::endl;
            cv::Mat_<float> ms(
                bw.cols - marker.cols + 1
              , bw.rows - marker.rows + 1
            );
            cv::matchTemplate(bw, marker, ms, CV_TM_CCORR_NORMED);
            cv_imshow(
                  img_prefix + "_mean_score.tif"
                , [&ms]
                {
                    cv::Mat tmp = ms.clone();
                    cv::normalize(tmp, tmp, 0, 65535, cv::NORM_MINMAX, CV_16U);
                    return tmp;
                }
                , verbose
            );
            if ( scores.empty() )
                scores = ms;
            else
                scores += ms;
        }

        struct Buf
        {
            const uint32_t& x_marker_num, y_marker_num;
            const uint32_t& marker_x_interval, marker_y_interval;
        } cml_buf 
        { 
            x_marker_num, y_marker_num, marker_x_interval, marker_y_interval
        };
        ChipMarkLayout<Buf> chip_marker_layout( scores, cml_buf );
        auto mrow = chip_marker_layout.get_layout_scores_n_rows();
        auto mcol = chip_marker_layout.get_layout_scores_n_cols();
        cpt::msg << VDUMP( mrow ) << std::endl;
        cpt::msg << VDUMP( mcol ) << std::endl;
        
        for ( decltype(mrow) r = 0; r != mrow; r ++ )
        {
            for ( decltype(mcol) c = 0; c != mcol; c ++ )
            {
                chip_marker_layout.push_score( r, c );
            }
        }
        auto s_p = chip_marker_layout.get_max_score_with_pos();
        auto& los = chip_marker_layout.get_layout_scores();

        // QC
        ROIQC qc;
        roi_qc_fail = !qc( 
              scores
            , cv::Point(s_p.j, s_p.i)
            , chip_marker_layout.get_layout_scores_n_cols()
            , chip_marker_layout.get_layout_scores_n_rows()
            , x_marker_num
            , y_marker_num
            , marker_x_interval
            , marker_y_interval
        );

        cv_imshow(
              img_prefix + "_layout_score.tif"
            , [&los]
            {
                cv::Mat tmp ( los.clone() );
                cv::normalize(tmp, tmp, 0, 65535, cv::NORM_MINMAX, CV_16U);
                return tmp;
            }
            , verbose
        );
        cpt::msg << VDUMP(s_p.score) << std::endl;
        // search max layout score
        return chip_marker_layout.get_bound_rect( s_p, markers.at(0) );
    }
    /// @private 
    static auto truncate( const std::vector<cv::Rect>& v, std::size_t col_nums
            , const cv::Rect& roi, const cv::Point& orgp )
    {
        std::vector<cv::Rect> res;
        return res;
    }
    /// @private 
    static void grid_normalize_tile( 
          std::vector<cv::Rect>& vec
        , std::size_t col_nums
        , const std::vector<uint32_t>& gl_x
        , const std::vector<uint32_t>& gl_y
        , const cv::Rect& roi
        , const std::string& origin_position
        , char x_axis_direction
    )
    {
        auto orgp = cv::Point( gl_x.at(roi.x), gl_y.at(roi.y) );
        CoordinateSystemNormalization csn;
        // vec = truncate( vec, col_nums, roi, orgp );
        {
            std::vector<cv::Rect> new_tiles;
            new_tiles.resize( roi.width * roi.height );
            auto&& rmv = cpt::view::make_row_major_view( vec, gl_x.size() - 1, gl_y.size() - 1 );
            auto&& new_rmv = cpt::view::make_row_major_view( new_tiles, roi.width, roi.height );
            for ( auto i = roi.y; i < roi.y + roi.height; i ++ )
            {
                for ( auto j = roi.x; j < roi.x + roi.width; j ++ )
                {
                    auto& t = rmv( i, j );
                    t.x -= orgp.x;
                    t.y -= orgp.y;
                    new_rmv(i - roi.y, j - roi.x) = t;
                }
            }
            vec.swap(new_tiles);
        }
        csn.operator()( vec, col_nums, origin_position, x_axis_direction );
    }
    /// @private 
    static auto get_marker_check_grid ( 
          uint32_t marker_x_interval
        , uint32_t marker_y_interval 
        , uint32_t marker_width
        , uint32_t marker_height
        , uint32_t x_marker_num
        , uint32_t y_marker_num
        , cv::Mat& mean
    )
    {
        auto grid_width  = x_marker_num * marker_width;
        auto grid_height = y_marker_num * marker_height;
        cv::Mat_<float> res (grid_height, grid_width); 
        for ( uint32_t i = 0; i < y_marker_num; i ++ )
        {
            for ( uint32_t j = 0; j < x_marker_num; j ++ )
            {
                auto smy = i * marker_y_interval;
                auto smx = j * marker_x_interval;
                auto dmy = i * marker_height;
                auto dmx = j * marker_width;
                mat_copy<float>( 
                      mean
                    , res
                    , cv::Rect( 
                        cv::Point( smx, smy )
                        , cv::Point( 
                              smx + marker_width
                            , smy + marker_height
                        )
                    )
                    , cv::Rect( 
                        cv::Point( dmx, dmy )
                        , cv::Point( 
                              dmx + marker_width
                            , dmy + marker_height
                        )
                    )
                );
                // cpt::msg << VDUMP( smy ) << std::endl;
                // cpt::msg << VDUMP( smx ) << std::endl;
                // cpt::msg << VDUMP( dmy ) << std::endl;
                // cpt::msg << VDUMP( dmx ) << std::endl;
            }
        }
        return res;
    }
  public:
    /**
     * @brief   The Regioin of Interest detection algorithm implementation.
     * @details Search the marker position by the given marker pattern "markers", bound the region coverred by all markers, and extract the intensities.
     * @param mean              The mean value of intensities grid.
     * @param stddev            The standard deviation of the intensities grid.
     * @param pixels            The pixel of raw image
     * @param cv_mat            The matrix of cv relate to every probe. ( can be empty matrix )
     * @param detail_raw_values The detail pixel values in every probe. ( can be empty vector )
     * @param roi_qc_fail       The output QC state of ROI process.
     * @param x_marker_num      The horizontal direction of marker numbers of the image ( mean intensity grid ).
     * @param y_marker_num      The vertical direction of marker numbers of the image ( mean intensity grid ).
     * @param marker_x_interval The grid distance ( cell numbers ) between markers of the horizontal direction.
     * @param marker_y_interval The grid distance ( cell numbers ) between markers of the vertical direction.
     * @param markers           The candidate marker patterns. The algorithm will search the marker pattern from the first, and if the match quality is too low, then it will try to match the next candidate of the markers. <BR>
     * The marder pattern is specified by opencv matrix with uint8_t integer element, for example : <BR>
     *  . . . . . . . . . . <BR> 
     *  . . . . . . . . . . <BR> 
     *  . . X X . . X X . . <BR> 
     *  . . X X . . X X . . <BR> 
     *  . . . . X X . . . . <BR> 
     *  . . . . X X . . . . <BR> 
     *  . . X X . . X X . . <BR> 
     *  . . X X . . X X . . <BR> 
     *  . . . . . . . . . . <BR> 
     *  . . . . . . . . . . <BR>
     *  The 'X' is 255. <BR>
     *  The '.' is 0. <BR>
     *  @param img_path          The filesystem path of raw image.
     *  @param enable            False then this function will do nothing, otherwise work normally
     *  @param v_mean_trimmed    Show trimmed mean image
     *  @param v_mean_binarized  Show binarized mean image
     *  @param v_mean_score      Show scored mean image
     *  @param v_layout_score    Show layout score image
     *  @param v_marker_check    Show marker check image
     *  @param v_mean            Show mean image
     *  @param v_std             Show standard deviation image
     *  @param v_cv              Show cv image
     *  @param verbose           Set to false if no image show process are need ( will override other "v_" prefix variable ), else set to true.
     */
    template<class TID, class GLID>
    auto operator() ( 
          const std::vector<cv::Mat>& stat
        , const TiledMat<TID, GLID>& tiled_src
        , const uint8_t& x_marker_num
        , const uint8_t& y_marker_num
        , const uint32_t& marker_x_interval
        , const uint32_t& marker_y_interval
        , const std::vector<cv::Mat_<uint8_t>>& markers
        // , const std::string& img_path
        // , const bool& enable
        // , const int16_t& v_mean_trimmed
        // , const int16_t& v_mean_binarized
        // , const int16_t& v_mean_score
        // , const int16_t& v_layout_score
        // , const int16_t& v_marker_check
        // , const int16_t& v_mean
        // , const int16_t& v_std
        // , const int16_t& v_cv
        // , const bool& verbose = true
    )
    {
        auto img_prefix = boost::filesystem::path( img_path ).stem().string() + "_roi";
        cv::Rect roi{
            0, 0, mean.cols, mean.rows
        };
        struct Result
        {
            decltype(mean.cols) feature_cols;
            decltype(mean.rows) feature_rows;
            decltype(roi) _roi;
        };        
        if (enable)
        {
            roi = ( get_layout_score(
                  mean
                , x_marker_num
                , y_marker_num
                , marker_x_interval
                , marker_y_interval
                , markers
                , roi_qc_fail
                , img_prefix
                , v_mean_trimmed
                , v_mean_binarized
                , v_mean_score
                , v_layout_score
                , verbose
            ) );
            cpt::msg << "============ extract rect ============ " << std::endl;
            cpt::msg << VDUMP(roi.x) << std::endl;
            cpt::msg << VDUMP(roi.y) << std::endl;
            cpt::msg << VDUMP(roi.width) << std::endl;
            cpt::msg << VDUMP(roi.height) << std::endl;
    
            std::cerr << roi << '\n';
            if ( detail_raw_values.size() > 0 )
            {
                vec_roi( detail_raw_values, mean.cols, roi );
            }
            mean = mean(roi);
            stddev = stddev(roi);
            pixels = pixels(roi);
            if ( cv_mat.cols > 0 && cv_mat.rows > 0 )
            {
                cv_mat = cv_mat(roi);
            }
            // bounded box
            Result res {mean.cols, mean.rows, roi};
            return res;
        }
        cv_imshow(
              img_prefix + "_marker_check.tif"
            , [&]
            {
                cpt::msg << "marker check grid" << std::endl;
                auto tmp = mean.clone();
                cv::Mat mcg = get_marker_check_grid( 
                      marker_x_interval
                    , marker_y_interval
                    , markers.at(0).cols
                    , markers.at(0).rows
                    , x_marker_num
                    , y_marker_num
                    , tmp
                );
                cv::normalize(mcg, mcg, 0, 65535, cv::NORM_MINMAX, CV_16U);
                return mcg;
            }
            , verbose
        );
    
        cv_imshow(
            img_prefix + "_mean.tif"
          , [&]
            {
                cv::Mat tmp = trim_outlier( mean.clone(), 0, 0.02 );
                // tmp = max_threshold( std::move(tmp), 500 );
                cv::normalize(tmp, tmp, 0, 65535, cv::NORM_MINMAX, CV_16U);
                return tmp;
            }
          , verbose
        );
        return Result
        {
            mean.cols, mean.rows, roi 
        };
    }
};

}
