#include <ChipImgProc/utils.h>
#include <algorithm>
#include <ChipImgProc/rotation/from_warp_mat.hpp>
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
        double          angle
    ) const {
        assert(!hints.empty());
        cv::Mat_<std::uint8_t> image;
        typed_mat(_image, [&image](auto&& mat){
            image = norm_u8(mat);
        });
        auto h = templ.rows;
        auto w = templ.cols;
        auto templ_center = cv::Point2d(
            (h - 1) / 2.0, 
            (w - 1) / 2.0
        );
        auto rot_mat = cv::getRotationMatrix2D(templ_center, angle, 1.0);
        templ = warp_affine_u8(templ, rot_mat, {w, h});
        mask = warp_affine_u8(mask, rot_mat, {w, h});

        auto score_matrix = chipimgproc::match_template(
            image, templ, cv::TM_CCORR_NORMED, mask
        );
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

        int x0 = std::floor(_x0 - templ_center.x);
        int y0 = std::floor(_y0 - templ_center.y);
        int x1 = std::ceil(_x1 + templ_center.x);
        int y1 = std::ceil(_y1 + templ_center.y);

        cv::Size patch_size(image.cols - x1 + x0, image.rows - y1 + y0);
        cv::Point2d patch_center((patch_size.width - 1) / 2.0, (patch_size.height - 1) / 2.0);
        cv::Mat scores(cv::Mat::zeros(patch_size, score_matrix.type()));

        for(auto&& h : hints) {
            cv::Point2f center(
                h.x - x0 - templ_center.x + patch_center.x, 
                h.y - y0 - templ_center.y + patch_center.y
            );
            cv::Mat tmp;
            cv::getRectSubPix(score_matrix,
                patch_size, center, tmp);
            scores += tmp;
        }
        cv::Point max_score_p;
        cv::minMaxLoc(scores, nullptr, nullptr, nullptr, &max_score_p);
        auto patch = scores(cv::Rect(
            max_score_p.x - 1,
            max_score_p.y - 1,
            3, 3
        ));
        std::vector<double> zx(3);
        std::vector<double> zy(3);

        for(int i = 0; i < patch.rows; i ++) {
            for(int j = 0; j < patch.cols; j ++) {
                auto n = patch.at<float>(i, j);
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

        return std::make_tuple(
            cv::Point2d(max_score_p.x + dx - x0, max_score_p.y + dy - y0), 
            scores.at<float>(max_score_p.y, max_score_p.x) / hints.size()
        );
    }
    auto operator()(
        const cv::Mat&  _image,
        cv::Mat         templ,
        cv::Mat         mask,
        const Hints&    hints,
        cv::Mat         warp_mat
    ) const {
        auto angle = rotation::from_warp_mat(warp_mat);
        return this->operator()(
            _image, templ, mask,
            hints, -angle
        );
    }
};

constexpr EstimateBias estimate_bias;

}