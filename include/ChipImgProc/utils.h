#pragma once
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
namespace chipimgproc { 
const char* depth(const cv::Mat& image);
void info(const cv::Mat& image);
constexpr double cmax(std::int32_t depth)
{
    switch (depth)
    {
        case 0 : return std::numeric_limits<std::uint8_t>::max();
        case 1 : return std::numeric_limits<std::int8_t>::max();
        case 2 : return std::numeric_limits<std::uint16_t>::max();
        case 3 : return std::numeric_limits<std::int16_t>::max();
        case 4 : return std::numeric_limits<std::int32_t>::max();
        default: return 1.0;
    }
}

double cmax(const cv::Mat& image);

std::string figure(
    const std::string& name
  , const int x = 0
  , const int y = 40
  , const int width  = 800
  , const int height = 600
); 
cv::Mat imread(const boost::filesystem::path& fname);

bool imwrite(const boost::filesystem::path& fname, const cv::Mat src);

template <class MAT>
MAT imresize(MAT&& src, const double scale)
{
    cv::resize(src, src, cv::Size(), scale, scale, cv::INTER_LINEAR);
    return std::forward<MAT>(src);
}

template <class MAT>
MAT imresize(MAT&& src, const cv::Size& dsize)
{
    cv::resize(src, src, dsize, 0, 0, cv::INTER_LINEAR);
    return std::forward<MAT>(src);
}

template <class MAT>
MAT imrotate(MAT&& src, const double angle, const double scale = 1)
{
    cv::warpAffine(
        src
      , src
      , cv::getRotationMatrix2D(
            cv::Point(src.cols >> 1, src.rows >> 1)
          , angle
          , scale
        )
      , src.size()
      , cv::INTER_LINEAR
      , cv::BORDER_CONSTANT
      , cv::Scalar(0)
    );
    return std::forward<MAT>(src);
}

void cv_windows_config( bool active = true );

template<class M>
auto trim_outlier( M&& mm, float ltrim_rate, float utrim_rate )
{
    auto m = mm.clone();
    auto size = m.rows * m.cols;
    typename std::decay_t<M>::value_type ltrim_size ( size * ltrim_rate );
    typename std::decay_t<M>::value_type utrim_size ( size * utrim_rate );

    std::nth_element ( m.begin(), m.begin() + ltrim_size, m.end());
    auto lower = *(m.begin() + ltrim_size);

    auto upper_offset = size - utrim_size - 1;
    std::nth_element ( m.begin(), m.begin() + upper_offset, m.end());
    auto upper = *(m.begin() + upper_offset);
    for ( auto&& v : mm )
    {
        if ( v > upper ) v = upper;
        if ( v < lower ) v = lower; 
    }
    return mm;
}
template<class T, class SM, class DM>
void mat_copy ( 
      const SM& sm
    , DM&& dm
    , const cv::Rect& sr
    , const cv::Rect& dr 
)
{
    assert ( sr.width == dr.width );
    assert ( sr.height == dr.height );
    for ( decltype(sr.height) i = 0; i < sr.height; i ++ )
    {
        for ( decltype(sr.width) j = 0; j < sr.width; j ++ )
        {
            assert( dr.y + i < dm.rows );
            assert( dr.x + j < dm.cols );
            assert( sr.y + i < sm.rows );
            assert( sr.x + j < sm.cols );
            dm.template at<T>( dr.y + i , dr.x + j ) 
                = sm.template at<T>( sr.y + i, sr.x + j );
        }
    }
}
cv::Mat gray_log( const cv::Mat& in, float trim_rate = 0.0 );

bool imwrite(const boost::filesystem::path& fname, const cv::Mat src, float trim_rate );
// template <class FUNC, class... ARGS>
// void cv_imshow(
//       int16_t delay
//     , FUNC&& func
//     , ARGS&&... args
//     , bool active = true
// )
// {
//     if (active and delay >= 0)
//     {
//         auto image = func(args...);
//         cv::imshow("verbose", image);
//         cv::waitKey(delay);
//     }
// }
// 
// template <class FUNC, class... ARGS>
// auto cv_imshow(
//       const std::string& name    
//     , FUNC&& func
//     , bool active = true
// )
// {
//     if (active )
//     {
//         auto image = func();
//         cv::imwrite( name, image );
//     }
// }

}