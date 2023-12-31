#pragma once
#include <boost/filesystem.hpp>
#include <ChipImgProc/utils/cv.h>
#include <ChipImgProc/utils/less.hpp>
#include <ChipImgProc/utils/mat.hpp>
#include <ChipImgProc/histogram.hpp>
#include <ChipImgProc/logger.hpp>
namespace chipimgproc { 

const double PI = std::atan(1.0)*4;

std::string depth(const cv::Mat& image);

template<class OS>
void info(OS& os, const cv::Mat& image)
{
    os       << "- dims       " << image.dims         << std::endl
             << "- rows       " << image.rows         << " elems" << std::endl
             << "- cols       " << image.cols         << " elems" << std::endl
             << "- total      " << image.total()      << " elems" << std::endl
             << "- channels   " << image.channels()   << std::endl
             << "- elemSize   " << image.elemSize()   << " bytes" << std::endl
             << "- elemSize1  " << image.elemSize1()  << " bytes" << std::endl
             << "- depth      " << depth(image)       << std::endl;
}
std::string info_str(const cv::Mat& image);
void info_log(const cv::Mat& image);
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

cv::Mat affine_resize(cv::Mat src, double fx, double fy = 0, int interplation = cv::INTER_AREA);

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
auto trim_outlier( M&& mm, int peek_threshold = 40000)
{
    auto px_count = mm.rows * mm.cols;
    int threshold = px_count / peek_threshold;
    auto limit = px_count / 10;
    auto hist = histogram(mm, 256);
    int i = -1;
    bool found = false;
    std::size_t count;
    do {
        i ++;
        count = hist[i].second;
        if( count > limit ) count = 0;
        found = count > threshold;
    } while(!found && i < (hist.size()-1));
    int hmin = i;
    i = hist.size();
    do {
        i --;
        count = hist[i].second;
        if(count > limit) count = 0;
        found = count > threshold;
    } while(!found && i > 0);
    int hmax = i;
    float lbound, ubound;
    if(hmax < hmin) hmax = hmin;
    lbound = hist.lbound + hmin * hist.bin_size;
    ubound = hist.lbound + hmax * hist.bin_size;
    lbound *= 0.5;  // (*)
    for(auto&& v : mm) {
        if(v < lbound) v = lbound;
        if(v > ubound) v = ubound;
    }
    return mm;
}
template<class T>
cv::Mat_<std::uint8_t> norm_u8(const cv::Mat_<T>& m, int peek_threshold = 40000) {
    auto trimmed_m = trim_outlier(m.clone(), peek_threshold); // TODO: smarter way
    cv::Mat_<std::uint8_t> bin;
    cv::normalize(trimmed_m, bin, 1, 255, cv::NORM_MINMAX, bin.depth()); // (*)
    return bin;
}
template<class T>
cv::Mat_<std::uint8_t> binarize(const cv::Mat_<T>& m, int peek_threshold = 40000) {
    auto trimmed_m = trim_outlier(m.clone(), peek_threshold); // TODO: smarter way
    cv::Mat_<std::uint8_t> bin(m.rows, m.cols);
    cv::threshold(trimmed_m, bin, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
    // cv::normalize(trimmed_m, bin, 0, 255, cv::NORM_MINMAX, bin.depth());
    return bin;
}

std::tuple<double, cv::Mat> threshold(cv::Mat src, double thresh, double maxval, int type);

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
cv::Mat gray_log( const cv::Mat& in, int peek_threshold = 40000);

bool imwrite(
    const boost::filesystem::path& fname, 
    const cv::Mat src, 
    int peek_threshold = 40000
);

cv::Mat_<std::uint16_t> viewable(
    const cv::Mat& m, 
    int peek_threshold = 40000
);

using ViewerCallback = std::function<void(const cv::Mat&)>;
template<class... ARGS>
using ViewerCallbackA = std::function<void(const cv::Mat&, ARGS...)>;

int depth_to_bits_num( const cv::Mat& m );

cv::Rect bound_rect( const std::vector<cv::Point>& points ); 

std::string jpg_base64( const cv::Mat& pixels);

cv::Mat_<float> match_template(
    cv::Mat img, cv::Mat tpl, 
    cv::TemplateMatchModes mode = cv::TM_CCORR_NORMED,
    cv::InputArray mask = cv::noArray()
);

template<class Func>
void typed_mat(cv::Mat& mat, Func&& func) {
    switch (mat.depth())
    {
        case 0: 
            func(static_cast<cv::Mat_<std::uint8_t>&>(mat));
            break;
        case 1: 
            func(static_cast<cv::Mat_<std::int8_t>&>(mat));
            break;
        case 2: 
            func(static_cast<cv::Mat_<std::uint16_t>&>(mat));
            break;
        case 3: 
            func(static_cast<cv::Mat_<std::int16_t>&>(mat));
            break;
        case 4: 
            func(static_cast<cv::Mat_<std::int32_t>&>(mat));
            break;
        case 5: 
            func(static_cast<cv::Mat_<float>&>(mat));
            break;
        case 6: 
            func(static_cast<cv::Mat_<double>&>(mat));
            break;
        default:  
            throw std::runtime_error("undefined mat depth");
    }
}
template<class Func>
void typed_mat(const cv::Mat& mat, Func&& func) {
    switch (mat.depth())
    {
        case 0: 
            func(static_cast<const cv::Mat_<std::uint8_t>&>(mat));
            break;
        case 1: 
            func(static_cast<const cv::Mat_<std::int8_t>&>(mat));
            break;
        case 2: 
            func(static_cast<const cv::Mat_<std::uint16_t>&>(mat));
            break;
        case 3: 
            func(static_cast<const cv::Mat_<std::int16_t>&>(mat));
            break;
        case 4: 
            func(static_cast<const cv::Mat_<std::int32_t>&>(mat));
            break;
        case 5: 
            func(static_cast<const cv::Mat_<float>&>(mat));
            break;
        case 6: 
            func(static_cast<const cv::Mat_<double>&>(mat));
            break;
        default:  
            throw std::runtime_error("undefined mat depth");
    }
}
template<class T>
int type_to_depth() {
    return cv::Mat_<T>().depth();
}

cv::Mat filter2D(cv::Mat mat, cv::Mat kern, int ddepth = CV_64F);

void ip_convert(cv::Mat& mat, int type, double alpha = 1, double beta = 0);

}
#include "utils/kron.h"