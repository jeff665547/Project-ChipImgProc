#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/marker/layout.hpp>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <Nucleona/stream/null_buffer.hpp>
#include <ChipImgProc/rotation/calibrate.hpp>
namespace chipimgproc { namespace rotation {

// using FLOAT = float;
template<class FLOAT>
struct MarkerFit {

private:
    template<class T>
    auto mean( const std::vector<T>& d ) const {
        T sum = 0;
        for(auto& v : d) {
            sum += v;
        }
        return sum / d.size();
    }
    auto x_y_group( const std::vector<marker::detection::MKRegion>& set ) const {
        std::map<int, std::vector<cv::Point>> x_group, y_group;
        for(auto&& mk_r : set ) {
            x_group[mk_r.x_i].push_back(cv::Point(mk_r.x, mk_r.y));
            y_group[mk_r.y_i].push_back(cv::Point(mk_r.x, mk_r.y));
        }
        return nucleona::make_tuple(
            std::move(x_group), std::move(y_group)
        );
    }
    auto horizontal_estimate(std::map<int, std::vector<cv::Point>>& x_group) const {
        for( auto&& p : x_group ) {
            std::sort(p.second.begin(), p.second.end(), [](const auto& a, const auto& b){
                return a.y < b.y;
            });
            auto vec = p.second.back() - p.second.front();
        }
    }
public:
    auto operator()(
        const cv::Mat_<std::uint16_t>&          src                                     ,
        const marker::Layout&                   mk_layout                               ,
        const std::vector<
            marker::detection::MKRegion
        >&                                      mk_regions                              ,
        const FLOAT&                            min_theta                               ,
        const FLOAT&                            max_theta                               ,
        const int&                              steps                                   ,
        std::ostream&                           logger     = nucleona::stream::null_out ,
        const ViewerCallback&                   v_bin      = nullptr,
        const ViewerCallbackA<int,int,int>&     marker_rot = nullptr           
    ) const {
        auto[x_group, y_group] = x_y_group(mk_regions);
        auto thetas = horizontal_estimate(x_group);
    }
private:
    auto rotate(const cv::Mat& mat, FLOAT theta) const {
        cv::Mat res = mat.clone();
        rotator_(res, theta);
        return res;
    }
    double scoring( 
        const cv::Mat_<std::uint8_t>& mat,
        const std::vector<cv::Mat_<std::uint8_t>>& mks,
        FLOAT theta
    ) const {
        cv::Mat_<float> score_mat;
        for(auto&& mk_ : mks) {
            // cv::Mat_<std::uint8_t> mk = cv::Mat_<std::uint8_t>::ones(mk_.rows, mk_.cols);
            auto mk = rotate(mk_, theta);
            cv::imwrite("debug_mk_rot.tiff", mk);
            cv::Mat_<float> candi_score_mat(
                mat.rows - mk.rows + 1,
                mat.cols - mk.cols + 1
            );
            cv::matchTemplate(mat, mk, candi_score_mat, CV_TM_CCORR_NORMED);
            cv::imwrite("debug_score.tiff", viewable(candi_score_mat, 0, 0));
            if( score_mat.empty() ) {
                score_mat = candi_score_mat;
            } else {
                score_mat += candi_score_mat;
            }
        }
        double min, max;
        cv::Point min_loc, max_loc;
        cv::minMaxLoc(score_mat, &min, &max, &min_loc, &max_loc);
        return max;
    }
    Calibrate rotator_;
};

}}