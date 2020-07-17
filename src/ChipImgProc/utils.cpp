#include <ChipImgProc/utils.h>
#include <Nucleona/language.hpp>
#include <boost/filesystem.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <cpp_base64/base64.h>
#include <ChipImgProc/logger.hpp>
namespace chipimgproc { 
std::string info_str(const cv::Mat& image) {
    std::stringstream ss;
    info(ss, image);
    return ss.str();
}
void info_log(const cv::Mat& image) {
    log.debug(info_str(image));
}
int cols(const cv::Mat& m) {
    return m.cols;
}
int rows(const cv::Mat& m) {
    return m.rows;
}
cv::Mat get_roi( cv::Mat& m, const cv::Rect& r) {
    return m(r);
}
std::string depth(const cv::Mat& image)
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
        default: return "Undefined";
    }
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
    cv::namedWindow(name, cv::WINDOW_NORMAL);
    cv::moveWindow(name, x, y);
    cv::resizeWindow(name, width, height);
    return name;
}

cv::Mat imread(const boost::filesystem::path& fname)
{
    return cv::imread(
        fname.string()
      , cv::IMREAD_ANYCOLOR
      | cv::IMREAD_ANYDEPTH
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
cv::Mat gray_log( const cv::Mat& in, int peek_threshold)
{
    auto img = in.clone();
    cv::Mat res;
    img.convertTo( res, CV_32F );
    cv::log( res, res );
    if ( peek_threshold != 0 )
    {
        res = trim_outlier( 
            (cv::Mat_<float>&)res, peek_threshold 
        );
    }
    cv::normalize( res, res, 0, 65535, cv::NORM_MINMAX, CV_16U );
    return res;
}

bool imwrite(const boost::filesystem::path& fname, const cv::Mat src, int peek_threshold)
{ 
    auto img = gray_log( src, peek_threshold);
    return cv::imwrite(fname.string(), img);
}

cv::Mat_<std::uint16_t> viewable(
    const cv::Mat& m, 
    int peek_threshold
) {
    cv::Mat_<std::uint16_t> tmp;
    switch(m.depth()) {
        case CV_16U:
            tmp = m.clone();
            trim_outlier(tmp, peek_threshold);
            break;
        case CV_8U:
            m.convertTo(tmp, CV_16U, 256);
            trim_outlier(tmp, peek_threshold);
            break;
        case CV_32F:
            // std::cout << "viewable: detect float image, assume range is [0, 1]" << std::endl;
            m.convertTo(tmp, CV_16U, 65535);
            break;
        default:
            throw std::runtime_error("not support depth: " + std::string(depth(m)));
    }
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
cv::Mat_<float> match_template(
    cv::Mat img, cv::Mat tpl, 
    cv::TemplateMatchModes mode,
    cv::InputArray mask
) {
    cv::Mat_<float> sm(
        img.rows - tpl.rows + 1,
        img.cols - tpl.cols + 1
    );
    cv::matchTemplate(img, tpl, sm, mode, mask);
    return sm;
}

std::tuple<double, cv::Mat> threshold(cv::Mat src, double thresh, double maxval, int type) {
    cv::Mat res(src.rows, src.cols, src.type());
    auto v = cv::threshold(src, res, thresh, maxval, type);
    return {v, res};
}

cv::Mat filter2D(cv::Mat mat, cv::Mat kern, int ddepth) {
    cv::Mat dst(mat.size(), ddepth);
    // cv::filter2D(mat, dst, dst.depth(), kern, 
    //     cv::Point(0, 0), 0, cv::BORDER_CONSTANT
    // );
    // cv::Rect roi(0, 0, mat.cols - kern.cols + 1, mat.rows - kern.rows + 1);
    // cv::Mat tmp = dst(roi);
    // return tmp;
    cv::filter2D(mat, dst, dst.depth(), kern);
    return dst;
}

void ip_convert(cv::Mat& mat, int type, double alpha, double beta) {
    cv::Mat tmp;
    mat.convertTo(tmp, type, alpha, beta);
    mat = tmp;
}


}