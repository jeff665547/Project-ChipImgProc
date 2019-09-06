/**
 * @file reg_mat_infer.hpp
 * @author Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief @copybrief chipimgproc::marker::detection::RegMatInfer
 * 
 */
#pragma once
#include <vector>
#include <ChipImgProc/marker/detection/mk_region.hpp>
#include <Nucleona/stream/null_buffer.hpp>
namespace chipimgproc::marker::detection {

/**
 * @brief Inference the missing marker and standardize the marker position
 * 
 */
constexpr class RegMatInfer {
    void anchor_infer(std::size_t expect, std::vector<double>& ancs, std::ostream& out) const {
        if(ancs.size() > 1 && ancs.size() < expect) {
            std::size_t sum = 0;
            for(std::size_t i = 1; i < ancs.size(); i ++) {
                auto invl = ancs.at(i) - ancs.at(i-1);
                sum += invl;
            }
            auto mean = sum / (ancs.size() - 1);
            if(auto under_rate = ancs.front() / mean; under_rate > 1) {
                std::vector<double> tmp;
                for(std::size_t i = std::floor(under_rate); i > 0; i --) {
                    tmp.push_back(ancs.front() - i * mean);
                }
                ancs = std::move(tmp);
            }
            for(std::size_t i = ancs.size(); i < expect; i ++ ) {
                ancs.push_back(ancs.back() + mean);
            }
        }
        if(expect > 0 && ancs.size() != expect)
            out << "marker region inference failed, the detected markers are too few\n";
    }
public:
    /**
     * @brief Given existing marker regions and matrix row and column,
     *        inference the missing marker and re-position the markers to fit the regular matrix.
     * 
     * @tparam T By default, std::uint16_t. Deduced, The input image(for debug) value type.
     * @param mk_regs   Marker regions with missing or slightly not fit the regular matrix
     * @param rows      The expected marker layout matrix rows 
     * @param cols      The expected marker layout matrix columns 
     * @param src       The marker detected image source, unnecessary for inference, only for debug output.
     * @param out       Deprecated. Log message output.
     * @param v_marker  The debug image output callback, the callback form is void(const cv::Mat&) type.
     *                  Current implementation is show the marker segmentation location
     * @return auto     Deduced, usually std::vector<MKRegion>. New marker regions. 
     */
    template<class T = std::uint16_t>
    auto operator() ( 
        std::vector<MKRegion>&  mk_regs,
        std::size_t             rows = 0,
        std::size_t             cols = 0,
        const cv::Mat_<T>&      src = cv::Mat_<std::uint16_t>(), 
        std::ostream&           out = nucleona::stream::null_out,
        const ViewerCallback&   v_marker  = nullptr 
    ) const {
        std::vector<double> x_anchor;
        std::vector<double> y_anchor;

        auto x_group_p = MKRegion::x_group_points(mk_regs);
        for(auto& p : x_group_p) {
            auto& vec = p.second;
            std::size_t x_sum = 0;
            int num = 0;
            for(auto& mk : vec ) {
                x_sum += mk.x;
                num ++;
            }
            auto mean = (double)x_sum / num;
            x_anchor.push_back(mean);
        }

        auto y_group_p = MKRegion::y_group_points(mk_regs);
        for(auto& p : y_group_p) {
            auto& vec = p.second;
            std::size_t y_sum = 0;
            int num = 0;
            for(auto& mk : vec ) {
                y_sum += mk.y;
                num ++;
            }
            auto mean = (double)y_sum / num;
            y_anchor.push_back(mean);
        }

        double width = 0;
        double height = 0;
        for( auto&& mk : mk_regs) {
            width += mk.width;
            height += mk.height;
        }
        width /= mk_regs.size();
        height /= mk_regs.size();
        if(x_anchor.size() < cols) throw std::runtime_error(
            "anchor number not match, probably marker detection failed"
        );
        if(y_anchor.size() < rows) throw std::runtime_error(
            "anchor number not match, probably marker detection failed"
        );
        std::vector<MKRegion> new_mk_regs;
        for(int y_i = 0; y_i < y_anchor.size(); y_i ++ ) {
            for( int x_i = 0; x_i < x_anchor.size(); x_i ++ ) {
                MKRegion mkr; // TODO: score
                mkr.x_i = x_i;
                mkr.y_i = y_i;
                mkr.x = x_anchor[x_i];
                mkr.y = y_anchor[y_i];
                mkr.width = width;
                mkr.height = height;
                new_mk_regs.push_back(mkr);
            }
        }
        auto view = norm_u8(src); 
        if(v_marker && !src.empty()) {
            for(auto& mk_r : new_mk_regs) {
                cv::rectangle(view, mk_r, 128, 1);
            }
            v_marker(view);
        }
        return new_mk_regs;
    } 
}
/**
 * @brief The global funtor with RegMatInfer type
 * 
 */
reg_mat_infer;

}