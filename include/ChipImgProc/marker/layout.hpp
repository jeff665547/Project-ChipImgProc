#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/const.h>
#include <Nucleona/tuple.hpp>
namespace chipimgproc{ namespace marker{
struct Des {
friend struct Layout;
    const cv::Point& get_pos_cl() const {
        return pos_cl_;
    }
    const cv::Point& get_pos_px() const {
        return pos_px_;
    }
    const cv::Point& get_pos(const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::CELL:
                return pos_cl_;
            case MatUnit::PX:
                return pos_px_;
            default:
                throw std::runtime_error(
                    "get_pos, unsupported unit: " + unit.to_string()
                );
        }
    }
    cv::Point& get_pos(const MatUnit& unit) {
        const auto& this_ref = *this;        
        return const_cast<cv::Point&>(this_ref.get_pos(unit));
    }
    const auto& get_candi_mks( const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::CELL:
                return candi_mks_cl;
            case MatUnit::PX:
                return candi_mks_px;
            default:
                throw std::runtime_error(
                    "get_candi_mks, unsupported unit: " + unit.to_string()
                );
        }
    }
    const auto& get_candi_mks_mask_px() const {
        return candi_mks_px_mask;
    }
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_cl;
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_px;
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_px_mask;
private:
    cv::Point pos_cl_;
    cv::Point pos_px_;
};
struct Layout {
    enum PatternNum { // How many kind of marker patterns
        single, multi
    };
    enum DistForm { 
        reg_mat, random
    };
    const Des& get_marker_des(int r, int c) const {
        if( dist_form != reg_mat) {
            throw std::runtime_error("get_marker_des(r,c) only support reg_mat form");
        }
        return mks.at(mk_map(r, c));
    }
    Des& get_marker_des(int r, int c) {
        const auto& this_ref = *this;        
        return const_cast<Des&>(this_ref.get_marker_des(r, c));
    }
    void set_reg_mat_dist(
        int rows, int cols, 
        const cv::Point& min_p, 
        std::uint32_t invl_x_cl, std::uint32_t invl_y_cl,
        std::uint32_t invl_x_px, std::uint32_t invl_y_px

    ) {
        dist_form = reg_mat;
        mk_map = decltype(mk_map)(rows, cols);
        mks.resize(rows * cols);
        mks.at(0).pos_cl_ = min_p;
        for(int r = 0; r < rows; r ++ ) {
            for( int c = 0; c < cols; c ++ ) {
                decltype(mk_map)::value_type idx = r * cols + c;
                mk_map(r, c) = idx;
                mks.at(idx).pos_cl_.x = min_p.x + invl_x_cl * c;
                mks.at(idx).pos_cl_.y = min_p.y + invl_y_cl * r;
                mks.at(idx).pos_px_.x = min_p.x + invl_x_px * c;
                mks.at(idx).pos_px_.y = min_p.y + invl_y_px * r;
            }
        }
        mk_invl_x_cl = invl_x_cl;
        mk_invl_y_cl = invl_y_cl;

        mk_invl_x_px = invl_x_px;
        mk_invl_y_px = invl_y_px;
    }
    void set_single_mk_pat( 
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px_mask
    ) {
        pat_num = single;
        for( auto& mk : mks ) {
            mk.candi_mks_cl      = candi_pats_cl;
            mk.candi_mks_px      = candi_pats_px;
            mk.candi_mks_px_mask = candi_pats_px_mask;
        }
    } 
    void set_mk_pat_reg_mat( 
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl,
        const cv::Point& p_id 
    ) {
        if( pat_num != multi) {
            throw std::runtime_error("set_mk_pat_reg_mat is used in multi pattern type");
        }
        if( dist_form != reg_mat ) {
            throw std::runtime_error("set_mk_pat_reg_mat is used in regular matrix");
        }
        get_marker_des(p_id.y, p_id.x).candi_mks_px = candi_pats_px;
        get_marker_des(p_id.y, p_id.x).candi_mks_cl = candi_pats_cl;
    }
    void set_mk_pat_random( 
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl,
        const cv::Point& p_px, 
        const cv::Point& p_cl 
    ) {
        if( pat_num != multi) {
            throw std::runtime_error("set_mk_pat_random is used in multi pattern type");
        }
        if( dist_form != random ) {
            throw std::runtime_error("set_mk_pat_random is used in random marker distribution");
        }
        Des mk;
        mk.candi_mks_px = candi_pats_px;
        mk.candi_mks_cl = candi_pats_cl;
        mk.pos_px_ = p_px;
        mk.pos_cl_ = p_cl;
        mks.push_back(mk);
    }
    auto get_marker_width_cl() const {
        return mks.at(0).candi_mks_cl.at(0).cols;
    }
    auto get_marker_height_cl() const {
        return mks.at(0).candi_mks_cl.at(0).rows;
    }
    auto get_marker_width_px() const {
        return mks.at(0).candi_mks_px.at(0).cols;
    }
    auto get_marker_height_px() const {
        return mks.at(0).candi_mks_px.at(0).rows;
    }
    auto get_marker_height(const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::PX:
                return get_marker_height_px();
            case MatUnit::CELL:
                return get_marker_height_cl();
            default: 
                throw std::runtime_error(
                    "Layout::get_marker_width, unsupport type"
                );
        }
    }
    auto get_marker_width(const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::PX:
                return get_marker_width_px();
            case MatUnit::CELL:
                return get_marker_width_cl();
            default: 
                throw std::runtime_error(
                    "Layout::get_marker_width, unsupport type"
                );
        }
    }
    auto get_marker_invl(const MatUnit& unit) {
        switch(unit) {
            case MatUnit::PX:
                return nucleona::make_tuple(
                    nucleona::copy(mk_invl_x_px),
                    nucleona::copy(mk_invl_y_px)
                );
            case MatUnit::CELL:
                return nucleona::make_tuple(
                    nucleona::copy(mk_invl_x_cl),
                    nucleona::copy(mk_invl_y_cl)
                );
            default:
                throw std::runtime_error(
                    "get_mk_invl, unsupported unit: " + unit.to_string()
                );
        }
    }
    auto get_marker_invl(const MatUnit& unit) const {
        return const_cast<Layout&>(*this).get_marker_invl(unit);
    }

    auto get_marker_rects(const MatUnit& unit) const {
        std::vector<cv::Rect> res;
        for( auto&& mk : mks ) {
            auto&& mk_mat = mk.get_candi_mks(unit).at(0);
            auto&& mk_pos = mk.get_pos(unit);
            res.push_back(
                cv::Rect(
                    mk_pos.x, mk_pos.y,
                    mk_mat.cols, mk_mat.rows
                )
            );
        }
        return res;
    }

    PatternNum              pat_num        { multi  } ;
    DistForm                dist_form      { random } ;
    cv::Mat_<std::int16_t>  mk_map                    ; // used in reg_mat
    std::vector<Des>        mks                       ; // used in all type
    std::uint32_t           mk_invl_x_cl              ; // used in reg_mat
    std::uint32_t           mk_invl_y_cl              ; // used in reg_mat
    std::uint32_t           mk_invl_x_px              ; // used in reg_mat
    std::uint32_t           mk_invl_y_px              ; // used in reg_mat
};


}}