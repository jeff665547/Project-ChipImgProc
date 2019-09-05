/**
 *  @file    ChipImgProc/gridding.hpp
 */
#pragma once
#include <opencv2/core/core.hpp>
#include <ChipImgProc/utils.h>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc
{


/**
 *  @brief Recognize the grid border of the image.
 *  @details Detail information can see here @ref improc_gridding
 */
template<class FLOAT>
struct Gridding
{
    struct Result {
        std::uint16_t feature_rows, feature_cols;
        std::vector<cv::Rect> tiles;
        std::vector<std::uint32_t> gl_x, gl_y;
    };
    // using FLOAT = float;
    template <int32_t dim, int32_t lg2nfft>
    auto fit_sinewave(
          const cv::Mat_<FLOAT>& src
        , const double max_invl
        , std::ostream& out = nucleona::stream::null_out
    )
    {
        // 1D projection
        cv::Mat_<FLOAT> data;
        cv::reduce(src, data, dim, cv::REDUCE_AVG, data.depth());
        if (dim == 1)
            cv::transpose(data, data);
    
        // Moving average
        cv::Mat_<FLOAT> ac;
        cv::copyMakeBorder(
            data -= cv::mean(data)
          , ac
          , 0.0
          , 0.0
          , 0.0
          , (1 << lg2nfft) - data.total()
          , cv::BORDER_CONSTANT
        );
        
        // Frequency detection
        cv::Mat_<cv::Point_<FLOAT>> ft;
        cv::dft(ac, ft, cv::DFT_COMPLEX_OUTPUT);
        FLOAT max = 0.0;
        FLOAT loc = 0.0;
        auto last = ft.total() >> 1;
        for (auto i = decltype(last)(ft.total()/max_invl); i != last; ++i)
        {
            auto val = cv::norm(ft(i));
            if (max < val)
            {
                max = val;
                loc = i;
            }
        }
        double freq = loc / static_cast<FLOAT>(ft.total());
        out << "invl = " << 1.0 / freq << '\n';
    
        // Phase estimation
        cv::Mat_<FLOAT> Phi, w;
        const auto v = 2.0 * CV_PI * freq;
        for (decltype(data.total()) i = 0; i != data.total(); ++i)
        {
            cv::Mat_<FLOAT> phi = (
                decltype(phi)(1, 3)
                <<  std::cos(v * i)
                  , std::sin(v * i)
                  , 1.0
            );
            Phi.push_back(phi);
        }
        cv::transpose(data, data);
        cv::solve(Phi, data, w, cv::DECOMP_NORMAL);
        const double phase = std::atan2(w(0), w(1)) * 0.5 / CV_PI + 0.25;
        out << "phase = " << phase << '\n';
        
        // Generate gridlines
        std::vector<FLOAT> anchors;
        int32_t start = std::ceil(phase);
        int32_t end = std::floor(freq * data.total() + phase);
        while (std::round((start - 1 - phase) / freq) > 0) --start;
        while (std::round((end - phase) / freq) < data.total()) ++end;
        for (auto i = start; i != end; ++i)
            anchors.emplace_back((i - phase) / freq);
    
        return anchors;
    }
        
    /**
     *  @brief  Recognize the grid border of the image.
     *  @param  in_src       Input image
     *  @param  max_invl    Max interval of grid line
     *  @param  verbose Set to false if no image shown are needed ( will override other "v_" prefix variable ), else set to true.
     *  @return grid rows, grid cols, grid tiles ( a rectangle set )
     */
    Result operator()( 
          const cv::Mat&            in_src
        , double                    max_invl 
        , std::ostream&             msg                 = nucleona::stream::null_out
        , const std::function<
            void(const cv::Mat&)
          >&                        v_result            = nullptr
    )
    {
        auto src = in_src.clone();
        std::vector<FLOAT> x, y;
        x = fit_sinewave<0,18>(src, max_invl, msg);
        y = fit_sinewave<1,18>(src, max_invl, msg);
        // if( !x.empty() ) {
        //     float ratio = ( x_.size() / (float)x.size() );
        //     msg << "last x num: "    << x.size()  << std::endl;
        //     msg << "current x num: " << x_.size() << std::endl;
        //     msg << "ratio: " << ratio << std::endl;
        //     if( ratio > 1.6 ) {
        //         break;
        //     } 
        // }
    
        std::vector<cv::Rect> tiles;
        for (decltype(y.size()) j = 1; j != y.size(); ++j)
        {
            const int y1 = std::round(y[j - 1]);
            const int y2 = std::round(y[j]);
            for (decltype(x.size()) i = 1; i != x.size(); ++i)
            {
                const int x1 = std::round(x[i - 1]);
                const int x2 = std::round(x[i]);
                tiles.emplace_back(
                    cv::Point(x1, y1)
                  , cv::Point(x2, y2)
                );
            }
        }
        std::vector<std::uint32_t> gl_x, gl_y;
        for ( auto&& xi : x ) 
            gl_x.emplace_back ( std::round(xi) );
        for ( auto&& yi : y )
            gl_y.emplace_back ( std::round(yi) );
        std::uint16_t feature_rows = y.size() - 1;
        std::uint16_t feature_cols = x.size() - 1;
    
        msg << "feature rows = " << feature_rows << '\n';
        msg << "feature cols = " << feature_cols << '\n';
    
        // draw gridding result
        if(v_result) {
            cv::Mat_<std::uint16_t> debug_img = viewable(in_src);
            auto color = 65536/2;
            for (auto tile: tiles)
            {
                tile.width  += 1;
                tile.height += 1;
                cv::rectangle(debug_img, tile, color);
            }
            v_result(debug_img);
        }
        Result res { 
              feature_rows
            , feature_cols 
            , std::move( tiles )
            , std::move( gl_x )
            , std::move( gl_y )
        };
        return res;
    }
};
}
