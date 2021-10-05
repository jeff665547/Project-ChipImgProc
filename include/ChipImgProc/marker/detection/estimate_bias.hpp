#include <ChipImgProc/utils.h>
#include <algorithm>
#include <ChipImgProc/rotation/from_warp_mat.hpp>
#include <cmath>
// #include <iostream>
namespace chipimgproc::marker::detection {

struct EstimateBias {
private:
    using Hints = std::vector<cv::Point2d>;
    auto warp_affine_u8(
        const cv::Mat& in, 
        const cv::Mat& trans_m, 
        cv::Size       dsize
    ) const {
        cv::Mat tmp, res;
        cv::warpAffine(in, tmp, trans_m, dsize);
        tmp.convertTo(res, CV_8U);
        return res;
    }
public:
    auto operator()(
        const cv::Mat&  _image,
        cv::Mat         templ,
        cv::Mat         mask,
        const Hints&    hints,
        double          angle,
        bool            global_search,  //TODO May be set as temaplte parameter
        cv::Size2d      local_cover_size,
        bool            regulation, //TODO May be set as temaplte parameter
        cv::Size2d      regulation_cover_size
    ) const {
        assert(!hints.empty());
        cv::Mat_<std::uint8_t> image;
        typed_mat(_image, [&image](auto&& mat){
            image = norm_u8(mat);
        });
        auto h = templ.rows;
        auto w = templ.cols;
        auto templ_center = cv::Point2d(
            (w - 1) / 2.0,
            (h - 1) / 2.0
        );
        auto rot_mat = cv::getRotationMatrix2D(templ_center, angle, 1.0);
        templ = warp_affine_u8(templ, rot_mat, {w, h});
        mask = warp_affine_u8(mask, rot_mat, {w, h});

        int x0, y0, x1, y1;
        cv::Size2d cover_size;        

        if (!global_search) {
            // Substitutional: Scan fluor marks only in the estimated position (region) with high likelihood (99.51% up). (*)
            cover_size.width = local_cover_size.width + w - 1;
            cover_size.height = local_cover_size.height + h - 1;
        }
        else {
            // Original: Scan fluor marks in all possible position (region).
            double _x0(hints.at(0).x);
            double _y0(hints.at(0).y); 
            double _x1(hints.at(0).x);
            double _y1(hints.at(0).y);

            for(auto&& p : hints) {
                _x0 = std::min(_x0, p.x);
                _y0 = std::min(_y0, p.y);
                _x1 = std::max(_x1, p.x);
                _y1 = std::max(_y1, p.y);
            }

            x0 = std::floor(_x0 - templ_center.x);
            y0 = std::floor(_y0 - templ_center.y);
            x1 = std::ceil(_x1 + templ_center.x);
            y1 = std::ceil(_y1 + templ_center.y);
            cover_size = cv::Size2d(image.cols - x1 + x0, image.rows - y1 + y0);
        }

        cv::Mat scores;
        cv::Point2d cover_center((cover_size.width - 1) / 2.0, (cover_size.height - 1) / 2.0);
        cv::Point2d scores_center((local_cover_size.width - 1) / 2.0, (local_cover_size.height - 1) / 2.0);
        if (!global_search) {
            scores.create(local_cover_size, cv::Mat1f().type());
            scores = cv::Scalar(0);
            // Substitutional: Substitutional cover center (for match_template score domain) (*)
            cv::Point2f center;
            cv::Mat cover;
            // cv::Mat score;
            for (auto&& h : hints) {
                center.x = h.x;
                center.y = h.y;
                // center.x = h.x - templ_center.x;
                // center.y = h.y - templ_center.y;
                cv::getRectSubPix(image, cover_size, center, cover);
                // score = chipimgproc::match_template(cover, templ, cv::TM_CCORR_NORMED, mask);
                // cv::Mat tmp(score.size(), CV_8U);
                // score.convertTo(tmp, CV_8U, 255);
                // cv::imwrite("score-" + std::to_string(h.x) + "-" + std::to_string(h.y) + ".tiff", tmp);
                // scores += score;
                scores += chipimgproc::match_template(cover, templ, cv::TM_CCORR_NORMED, mask);
            }
        }
        else {
            // Original: Original cover center (for match_template score domain) (*)
            auto score_matrix = chipimgproc::match_template(image, templ, cv::TM_CCORR_NORMED, mask);
            cv::Point2f center;

            scores.create(cover_size, cv::Mat1f().type());
            scores = cv::Scalar(0);
            for(auto&& h : hints) {
                center.x = h.x - x0 - templ_center.x + cover_center.x;
                center.y = h.y - y0 - templ_center.y + cover_center.y;

                cv::Mat score;
                cv::getRectSubPix(score_matrix, cover_size, center, score);
                // cv::Mat tmp(score.size(), CV_8U);
                // score.convertTo(tmp, CV_8U, 255);
                // cv::imwrite("score-" + std::to_string(h.x) + "-" + std::to_string(h.y) + ".tiff", tmp);
                scores += score;
            }
        }
        // cv::imwrite("score.tiff", scores);

        cv::Point max_score_p;
        cv::minMaxLoc(scores, nullptr, nullptr, nullptr, &max_score_p);

        // Sub-pixel position bias estimated by Gaussian function
        // auto cover = scores(cv::Rect(
        //     max_score_p.x - 1,
        //     max_score_p.y - 1,
        //     3, 3
        // ));
        cv::Mat cover;
        cv::getRectSubPix(scores, cv::Size2d(3.0, 3.0), max_score_p, cover);
        std::vector<double> zx(3);
        std::vector<double> zy(3);

        for(int i = 0; i < cover.rows; i ++) {
            for(int j = 0; j < cover.cols; j ++) {
                auto n = cover.at<float>(i, j);
                zx[j] += n;
                zy[i] += n;
            }
        }
        for(auto& n : zx) {
            n = std::log(n / 3);
        }
        for(auto& n : zy) {
            n = std::log(n / 3);
        }
        double dx = (0.5 * (zx[0] - zx[2])) / (zx[2] - (2 * zx[1]) + zx[0]);
        double dy = (0.5 * (zy[0] - zy[2])) / (zy[2] - (2 * zy[1]) + zy[0]);

        // Final estimation for bias correction
        cv::Point2d bias;
        if(regulation){
            // Substitutional: Bias correction testing for no adjustment case & high precision case (estimated position (region (95.6% up))).
            cv::Point2d rel_max_score_p, abs_rel_max_score_p;
            cv::Size2d regulation_cover_radius;
            rel_max_score_p.x = max_score_p.x - scores_center.x;
            rel_max_score_p.y = max_score_p.y - scores_center.y;
            abs_rel_max_score_p.x = std::abs(rel_max_score_p.x);
            abs_rel_max_score_p.y = std::abs(rel_max_score_p.y);
            regulation_cover_radius.width = regulation_cover_size.width / 2.0;
            regulation_cover_radius.height = regulation_cover_size.height / 2.0;
            
            if(abs_rel_max_score_p.x > regulation_cover_radius.width || abs_rel_max_score_p.y > regulation_cover_radius.height){
                bias.x = 0.0;
                bias.y = 0.0;
            }else{
                bias.x = rel_max_score_p.x + dx;
                bias.y = rel_max_score_p.y + dy;
            }
        }else if(!global_search){
            // Substitutional: Bias correction for substitutional case (estimated position (region (99.51% up))).
            bias.x = max_score_p.x + dx - scores_center.x;
            bias.y = max_score_p.y + dy - scores_center.y;
        }else{
            // Original: Bias correction for original case (all possible position (region)).
            bias.x = 0 + max_score_p.x + dx - x0;
            bias.y = 0 + max_score_p.y + dy - y0;
        }

        return std::make_tuple(
            bias, 
            scores.at<float>(max_score_p.y, max_score_p.x) / hints.size()
        );
    }
    auto operator()(
        const cv::Mat&  _image,
        cv::Mat         templ,
        cv::Mat         mask,
        Hints           hints_rum,
        cv::Mat         warp_mat,
        bool            global_search             = false,
        cv::Size2d      basic_cover_size          = cv::Size2d(0.0, 0.0),
        double          high_P_cover_extend_r     = 0.0,
        bool            regulation                = false,
        double          regulation_cover_extend_r = 0.0
    ) const {
        for(auto& h : hints_rum) {
            h.x -= 0.5;
            h.y -= 0.5;
        }
        std::vector<cv::Point2d> hints_dst(hints_rum.size());
        cv::transform(hints_rum, hints_dst, warp_mat);
        auto angle = rotation::rot_deg_from_warp(warp_mat);
        
        // Use different estimate cover and regulation for different scan mode.
        cv::Size2d regulation_cover_size;
        cv::Size2d cover_size;
        
        if(regulation){
            regulation_cover_size.width = regulation_cover_extend_r * templ.cols;
            regulation_cover_size.height = regulation_cover_extend_r * templ.rows;
        }
        
        cover_size.width = std::ceil(std::max(basic_cover_size.width, high_P_cover_extend_r * templ.cols));
        cover_size.height = std::ceil(std::max(basic_cover_size.height, high_P_cover_extend_r * templ.rows));

        return this->operator()(
            _image, templ, mask,
            hints_dst, -angle,
            global_search,
            cover_size,
            regulation,
            regulation_cover_size
        );
    }
};

constexpr EstimateBias estimate_bias;

}
