#include <ChipImgProc/utils.h>
#include <Nucleona/language.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cpp_base64/base64.h>
namespace chipimgproc { 
int cols(const cv::Mat& m) {
    return m.cols;
}
int rows(const cv::Mat& m) {
    return m.rows;
}
const char* depth(const cv::Mat& image)
{
    switch (image.depth())
    {
        case 0: return "CV_8U" ;
        case 1: return "CV_8S" ;
        case 2: return "CV_16U";
        case 3: return "CV_16S";
        case 4: return "CV_32S";
        case 5: return "CV_32F";
        case 6: return "CV_64F";
    }
    return "Undefined";
}

double cmax(const cv::Mat& image)
{
    return cmax(image.depth());
}

std::string figure(
    const std::string& name
  , const int x
  , const int y
  , const int width 
  , const int height
) {
    cv::namedWindow(name, ::CV_WINDOW_NORMAL);
    cv::moveWindow(name, x, y);
    cv::resizeWindow(name, width, height);
    return name;
}

cv::Mat imread(const boost::filesystem::path& fname)
{
    return cv::imread(
        fname.string()
      , ::CV_LOAD_IMAGE_ANYCOLOR
      | ::CV_LOAD_IMAGE_ANYDEPTH
    );
}

bool imwrite(const boost::filesystem::path& fname, const cv::Mat src)
{ 
    return cv::imwrite(fname.string(), src);
}

void cv_windows_config( bool active )
{
    if (active)
    {
        cv::namedWindow ("verbose", cv::WINDOW_NORMAL);
        cv::resizeWindow("verbose", 1600, 1200); 
        cv::moveWindow  ("verbose", 40 , 60 );
    }

}
cv::Mat gray_log( const cv::Mat& in, float trim_rate )
{
    auto img = in.clone();
    cv::Mat res;
    img.convertTo( res, CV_32F );
    cv::log( res, res );
    if ( trim_rate != (float)0.0 )
    {
        res = trim_outlier( 
            (cv::Mat_<float>&)res, trim_rate, 0 
        );
    }
    cv::normalize( res, res, 0, 65535, cv::NORM_MINMAX, CV_16U );
    return res;
}

bool imwrite(const boost::filesystem::path& fname, const cv::Mat src, float trim_rate )
{ 
    auto img = gray_log( src, trim_rate );
    return cv::imwrite(fname.string(), img);
}

cv::Mat_<std::uint16_t> viewable(
    const cv::Mat& m, 
    float ltrim, 
    float rtrim
) {
    cv::Mat_<std::uint16_t> tmp;
    switch(m.depth()) {
        case CV_16U:
            tmp = m.clone();
            break;
        case CV_8U:
            m.convertTo(tmp, CV_16U, 256);
            break;
        case CV_32F:
            // std::cout << "viewable: detect float image, assume range is [0, 1]" << std::endl;
            m.convertTo(tmp, CV_16U, 65535);
            break;
        default:
            throw std::runtime_error("not support depth: " + std::string(depth(m)));
    }
    trim_outlier(tmp, ltrim, rtrim);
    cv::Mat_<std::uint16_t> res = tmp.clone();
    cv::normalize( tmp, res, 0, 65535, cv::NORM_MINMAX );
    return res;
}

int depth_to_bits_num(const cv::Mat& m) {
    switch(m.depth()) {
        case 0: case 1: 
            return 8;
        case 2: case 3:
            return 16;
        case 4: case 5:
            return 32;
        case 6:
            return 64;
        default:
            throw std::runtime_error("undefined depth");
    }
}
cv::Rect bound_rect( const std::vector<cv::Point>& points ) {
    cv::Point max_p(
        std::numeric_limits<cv::Point::value_type>::min(),
        std::numeric_limits<cv::Point::value_type>::min()
    );
    cv::Point min_p(
        std::numeric_limits<cv::Point::value_type>::max(), 
        std::numeric_limits<cv::Point::value_type>::max() 
    );
    for (auto&& p : points ) {
        if( p.x > max_p.x) max_p.x = p.x;
        if( p.y > max_p.y) max_p.y = p.y;
        if( p.x < min_p.x) min_p.x = p.x;
        if( p.y < min_p.y) min_p.y = p.y;
    }
    return cv::Rect(
        min_p.x, min_p.y,
        max_p.x - min_p.x,
        max_p.y - min_p.y
    );
}
std::string jpg_base64( const cv::Mat& pixels) {
    std::vector<std::uint8_t> buf;
    cv::imencode(".jpg", pixels, buf);
    std::string pixels_encode = base64_encode(buf.data(), buf.size());
    // std::cout << "base64 size: " << pixels_encode.size() << std::endl;
    // std::cout << "content: " << pixels_encode << std::endl;
    return pixels_encode;
}


}