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
        cv::Mat_<std::uint8_t> src_u8;
        src.convertTo(src_u8, src_u8.type(), 0.00390625);
        src_u8 = binarize(src_u8);
        auto delta = (max_theta - min_theta) / steps;
        std::vector<FLOAT> thetas;
        thetas.reserve(mk_regions.size());
        for(auto& mk_r : mk_regions) {
            FLOAT max_score       = std::numeric_limits<FLOAT>::min();
            FLOAT max_score_theta = 0;
            auto&& candi_mks = mk_layout.get_marker_des(
                mk_r.y_i, mk_r.x_i
            ).get_candi_mks(MatUnit::PX);
            cv::Rect search_range;
            search_range.x = mk_r.x - (mk_r.width  / 2);
            if( search_range.x < 0 ) search_range.x = 0;
            search_range.y = mk_r.y - (mk_r.height / 2);
            if( search_range.y < 0 ) search_range.y = 0;
            search_range.width  = mk_r.width  * 2;
            search_range.height = mk_r.height * 2;
            auto mat = src_u8(search_range);
            cv::imwrite("debug_img_roi.tiff", mat);
            FLOAT theta = min_theta; // TODO: use range
            for( int s = 0; s < steps; s ++ ) {
                auto score = scoring(mat, candi_mks, theta);
                if( max_score < score ) {
                    max_score = score;
                    max_score_theta = theta;
                }
                theta += delta;
            }
            thetas.push_back(max_score_theta);
            logger << "mk(" << mk_r.x_i << "," << mk_r.y_i << "), theta: "
                << max_score_theta << ", score: " << max_score << std::endl;
            if( marker_rot ) {
                int mk_i = 0; // TODO: range
                for( auto&& mk : candi_mks ) {
                    marker_rot(rotate(mk, max_score_theta), mk_r.x_i, mk_r.y_i, mk_i);
                    mk_i ++; 
                }
            }
        }
        return mean(thetas);
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