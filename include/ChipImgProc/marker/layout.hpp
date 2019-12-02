/**
 * @file    layout.hpp
 * @author  Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief   @copybrief chipimgproc::marker::Layout
 */
#pragma once
#include <ChipImgProc/utils.h>
#include <ChipImgProc/const.h>
#include <Nucleona/tuple.hpp>
#include <ChipImgProc/marker/txt_to_img.hpp>
#include <range/v3/all.hpp>
namespace chipimgproc{ namespace marker{
/**
 * @brief Marker description, contains a series of candidate marker/mask patterns
 *        and logical position in cell/pixel level
 * 
 */
struct Des {
friend struct Layout;
    /**
     * @brief   Get position, in px or cell level
     * 
     * @param   unit PX/CELL
     * @return  const cv::Point& logical marker position 
     */
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
    /**
     * @brief   Get position, in px or cell level
     * 
     * @param   unit PX/CELL
     * @return  const cv::Point& logical marker position 
     */
    cv::Point& get_pos(const MatUnit& unit) {
        const auto& this_ref = *this;        
        return const_cast<cv::Point&>(this_ref.get_pos(unit));
    }
    /**
     * @brief Get all candidate markers in this description
     * 
     * @param unit PX/CELL 
     * @return const auto& deduced, usually std::vector<cv::Mat_<std::uint8_t>>
     */
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
    /**
     * @brief Get all candidate markers in this description
     * 
     * @param unit PX/CELL 
     * @return const auto& deduced, usually std::vector<cv::Mat_<std::uint8_t>>
     */
    const auto& get_candi_mks_mask( const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::CELL:
                return candi_mks_cl_mask;
            case MatUnit::PX:
                return candi_mks_px_mask;
            default:
                throw std::runtime_error(
                    "get_candi_mks, unsupported unit: " + unit.to_string()
                );
        }
    }
    /**
     * @brief Get the best match marker in current candidate marker list,
     *        The best match marker usually set by marker detection.
     * 
     * @param unit PX/CELL level marker pattern
     * @return const auto& The marker pattern, type deduced usually cv::Mat_<std::unit8_t>
     */
    const auto& get_best_mk(const MatUnit& unit) const {
        return get_candi_mks(unit).at(best_mk_idx);
    }
    /**
     * @brief Get the standard marker pattern, may different from best marker,
     *        because the input image quality may not always as good as expect.
     * 
     * @param unit PX/CELL level marker pattern
     * @return const auto& The marker pattern, type deduced usually cv::Mat_<std::unit8_t>
     */
    const auto& get_std_mk(const MatUnit& unit) const {
        return get_candi_mks(unit).at(0);
    }
    /**
     * @brief Get the best match marker related mask in current candidate marker list,
     *        The best match marker usually set by marker detection.
     * 
     * @param unit PX/CELL level marker pattern
     * @return const auto& The marker pattern, type deduced usually cv::Mat_<std::unit8_t>
     */
    const auto& get_best_mk_mask(const MatUnit& unit) const {
        return get_candi_mks_mask(unit).at(best_mk_idx);
    }
    /**
     * @brief Get the standard marker related mask, may different from best marker,
     *        because the input image quality may not always as good as expect.
     * 
     * @param unit PX/CELL level marker pattern
     * @return const auto& The marker pattern, type deduced usually cv::Mat_<std::unit8_t>
     */
    const auto& get_std_mk_mask(const MatUnit& unit) const {
        return get_candi_mks_mask(unit).at(0);
    }
    /**
     * @brief cell level candidate markers
     */
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_cl;
    /**
     * @brief pixel level candidate markers
     */
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_px;
    /**
     * @brief cell level candidate marker masks
     */
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_cl_mask; 
    /**
     * @brief pixel level candidate marker masks
     */
    std::vector<cv::Mat_<std::uint8_t>> candi_mks_px_mask;
    /**
     * @brief best marker index in candidate list
     */
    std::size_t best_mk_idx {0};
private:
    cv::Point pos_cl_;
    cv::Point pos_px_;
};
/**
 * @brief The Layout class is a description structure of marker style and marker placement in the physical design of the chip.
 *        It defines the marker positions in logical level, indexing by rows and columns of the regular matrix.
 *        Given the GDS file of the chip, miscellaneous sizes and relative locations of the markers can also 
 *        be specified in cell-level and pixel-level respectively. Currently, this class only supports 
 *        homogeneous marker style and regular grid-distributed layout type.
 *        The ArUco marker whose patterns are distinct from each other is not supported by this class.
 *
 * @details The following shows the examples of layout generation
 *      @snippet ChipImgProc/make_layout.hpp usage
 */
struct Layout {
friend struct MakeSinglePatternRegMatLayout;
    /**
     * @brief Marker pattern style number, currently only support single
     */
    enum PatternNum { // How many kind of marker patterns
        single, multi
    };
    /**
     * @brief Marker position distribution, currently on support reg_mat
     */
    enum DistForm { 
        reg_mat, random
    };
    /**
     * @brief Get the marker description by position.
     *        For a FOV with 3*3 markers layout, the position is (0,0), (0,1)...(2,2)
     * 
     * @param r The row position of marker
     * @param c The column position of marker
     * @return const Des& immutable description object
     */
    const Des& get_marker_des(int r, int c) const {
        if( dist_form != reg_mat) {
            throw std::runtime_error("get_marker_des(r,c) only support reg_mat form");
        }
        return mks.at(mk_map(r, c));
    }
    /**
     * @brief Get the marker description by position.
     *        For a FOV with 3*3 markers layout, the position is (0,0), (0,1)...(2,2)
     * 
     * @param r The row position of marker
     * @param c The column position of marker
     * @return Des& mutable description object
     */
    Des& get_marker_des(int r, int c) {
        const auto& this_ref = *this;        
        return const_cast<Des&>(this_ref.get_marker_des(r, c));
    }
    /**
     * @brief Get a marker description from layout. 
     *        The marker layout must be single pattern style.
     * 
     * @return const auto& Deduced, usually a Des type.
     */
    const auto& get_single_pat_marker_des() const {
        if(pat_num != single) {
            throw std::runtime_error(
                "get_single_pat_marker_des() only support single pattern"
            );
        }
        auto& mk = mks.at(0);
        return mk;
    }
    /**
     * @brief Get the candidate marker number.
     *        The marker must be single pattern style.
     * 
     * @return auto Deduced, usually std::size_t.
     */
    auto get_single_pat_candi_num() const {
        return get_single_pat_marker_des().candi_mks_cl.size();
    }
    /**
     * @brief Manually set the best marker index.
     *        Every marker description's best marker index wil be set to i.
     * 
     * @param i The best marker index.
     */
    void set_single_pat_best_mk(std::size_t i) {
        for(auto&& mk : mks) {
            mk.best_mk_idx = i;
        }
    }
    /**
     * @brief Set the marker layout to regular matrix distribution.
     * 
     * @param rows      Marker layout contained row number of markers.
     * @param cols      Marker layout contained col number of markers.
     * @param min_p     The first marker position related to origin, usually the marker have minimum position.
     * @param invl_x_cl The marker interval along X direction (horizontal) in cell level.
     * @param invl_y_cl The marker interval along Y direction (vertical) in cell level.
     * @param invl_x_px The marker interval along X direction (horizontal) in pixel level.
     * @param invl_y_px The marker interval along Y direction (vertical) in pixel level.
     */
    void set_reg_mat_dist(
        int rows, int cols, 
        const cv::Point& min_p, 
        std::uint32_t invl_x_cl, std::uint32_t invl_y_cl,
        std::uint32_t invl_x_px, std::uint32_t invl_y_px,
        std::uint32_t border_px
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
        this->border_px = border_px;
    }
    /**
     * @brief Set the marker layout to single marker pattern.
     * 
     * @param candi_pats_cl         Candidate marker pattern in cell level.
     * @param candi_pats_px         Candidate marker pattern in pixel level.
     * @param candi_pats_cl_mask    Candidate marker mask in cell level.
     * @param candi_pats_px_mask    Candidate marker mask in pixel level.
     */
    void set_single_mk_pat( 
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_cl_mask,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px_mask
    ) {
        pat_num = single;
        for( auto& mk : mks ) {
            mk.candi_mks_cl      = candi_pats_cl;
            mk.candi_mks_cl_mask = candi_pats_cl_mask;
            mk.candi_mks_px      = candi_pats_px;
            mk.candi_mks_px_mask = candi_pats_px_mask;
        }
    } 
    /**
     * @brief Get marker width in cell level.
     * 
     * @return auto Deduced, usually int.
     */
    auto get_marker_width_cl() const {
        return mks.at(0).candi_mks_cl.at(0).cols;
    }
    /**
     * @brief Get the marker height in cell level.
     * 
     * @return auto Deduced, usually int
     */
    auto get_marker_height_cl() const {
        return mks.at(0).candi_mks_cl.at(0).rows;
    }
    /**
     * @brief Get the marker width in pixel level.
     * 
     * @return auto Deduced, usually int
     */
    auto get_marker_width_px() const {
        return mks.at(0).candi_mks_px.at(0).cols;
    }
    /**
     * @brief Get the marker height in pixel level.
     * 
     * @return auto Deduced, usually int
     */
    auto get_marker_height_px() const {
        return mks.at(0).candi_mks_px.at(0).rows;
    }
    /**
     * @brief Get the marker height in given level
     * 
     * @param unit CELL/PX
     * @return auto Deduced, usually int
     */
    auto get_marker_height(const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::PX:
                return get_marker_height_px();
            case MatUnit::CELL:
                return get_marker_height_cl();
            default: 
                throw std::runtime_error(
                    "Layout::get_marker_width, unsupported type"
                );
        }
    }
    /**
     * @brief Get the marker width in given level
     * 
     * @param unit CELL/PX
     * @return auto Deduced, usually int
     */
    auto get_marker_width(const MatUnit& unit) const {
        switch(unit) {
            case MatUnit::PX:
                return get_marker_width_px();
            case MatUnit::CELL:
                return get_marker_width_cl();
            default: 
                throw std::runtime_error(
                    "Layout::get_marker_width, unsupported type"
                );
        }
    }
    /**
     * @brief Get the marker interval in given unit level
     * 
     * @param unit PX/CELL
     * @return auto Deduced, usually std::tuple<std::uint32_t, std::uint32_t>.
     *              Element 0 is X direction interval, 1 is Y direction interval.
     */
    auto get_marker_invl(const MatUnit& unit) const {
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
    /**
     * @brief Get the marker regions(rectangle) in given unit level
     * 
     * @param unit CELL/PX
     * @return auto Deduced, usually std::vector<cv::Rect>
     */
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
    
    /**
     * @brief Get the space between cells in pixel level
     */
    auto get_border_px(void) const {
        return this->border_px;
    }

    /**
     * @brief Pattern number type, multi or single.
     */
    PatternNum              pat_num        { multi  } ;

    /**
     * @brief Marker position distribution type, random or reg_mat.
     */
    DistForm                dist_form      { random } ;

    /**
     * @brief Regular matrix marker index map.
     */
    cv::Mat_<std::int16_t>  mk_map                    ; // used in reg_mat

    /**
     * @brief Marker descriptions
     */
    std::vector<Des>        mks                       ; // used in all type

    /**
     * @brief marker interval in cell level along the the X direction
     */
    std::uint32_t           mk_invl_x_cl              ; // used in reg_mat
    /**
     * @brief marker interval in pixel level along the the Y direction
     */
    std::uint32_t           mk_invl_y_cl              ; // used in reg_mat
    /**
     * @brief marker interval in pixel level along the the X direction
     */
    std::uint32_t           mk_invl_x_px              ; // used in reg_mat
    /**
     * @brief marker interval in pixel level along the the Y direction
     */
    std::uint32_t           mk_invl_y_px              ; // used in reg_mat

    /**
     * @brief space between cells in pixel level
     */
    std::uint32_t           border_px;
    
private:
    void reset_mk_pat(
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px,
        const std::vector<cv::Mat_<std::uint8_t>>& candi_pats_px_mask,
        std::uint32_t invl_x_px, std::uint32_t invl_y_px,
        std::uint32_t border_px = 0
    ) {
        auto& min_p = mks.at(0).pos_cl_;
        if(dist_form == reg_mat) {
            const auto& rows = mk_map.rows;
            const auto& cols = mk_map.cols;
            for(int r = 0; r < rows; r ++ ) {
                for( int c = 0; c < cols; c ++ ) {
                    decltype(mk_map)::value_type idx = r * cols + c;
                    mks.at(idx).pos_px_.x = min_p.x + invl_x_px * c;
                    mks.at(idx).pos_px_.y = min_p.y + invl_y_px * r;
                }
            }
            mk_invl_x_px = invl_x_px;
            mk_invl_y_px = invl_y_px;
            this->border_px = border_px;
        } else {
            throw std::runtime_error("dist_form: random not yet support");
        }
        for( auto& mk : mks ) {
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
};
/**
 * @brief Maker type of single pattern, regular matrix marker layout
 * 
 */
constexpr class MakeSinglePatternRegMatLayout
{
    auto to_px_domain(
        const std::vector<cv::Mat_<std::uint8_t>>& mks_cl,
        const std::vector<cv::Mat_<std::uint8_t>>& masks_cl,
        float cell_r_um,
        float cell_c_um,
        float border_um,
        std::uint32_t invl_x_cl, 
        std::uint32_t invl_y_cl,
        float um2px_r
    ) const {
        std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px;
        std::vector<cv::Mat_<std::uint8_t>> candi_mk_pats_px_mask;
        for(auto&& [mk, mask] : ranges::view::zip(mks_cl, masks_cl)) {
            auto [mk_img, mask_img] = txt_to_img_(
                mk, mask,
                cell_r_um * um2px_r,
                cell_c_um * um2px_r,
                border_um * um2px_r
            );
            candi_mk_pats_px.push_back(mk_img);
            candi_mk_pats_px_mask.push_back(mask_img);
        }
        std::uint32_t invl_x_px = std::round(invl_x_cl * (cell_c_um + border_um) * um2px_r); // can get this value from micron to pixel
        std::uint32_t invl_y_px = std::round(invl_y_cl * (cell_r_um + border_um) * um2px_r);
        std::uint32_t border_px = std::ceil(border_um * um2px_r);
        return std::make_tuple(
            candi_mk_pats_px,
            candi_mk_pats_px_mask,
            invl_x_px,
            invl_y_px, 
            border_px
        );

    }
public:
    /**
     * @brief Make maker from all needed information.
     * 
     * @param mks_cl        New marker pattern in cell level.
     * @param masks_cl      New marker mask in cell level.
     * @param cell_r_um     Cell height in micron.
     * @param cell_c_um     Cell width in micron.
     * @param border_um     Border size between cell in micron.
     * @param rows          Marker layout contained row number of markers.
     * @param cols          Marker layout contained col number of markers.
     * @param invl_x_cl     Marker interval along X direction in cell level. 
     * @param invl_y_cl     Marker interval along Y direction in cell level. 
     * @param um2px_r       Micron to pixel rate.
     * @return auto 
     */
    auto operator()(
        const cv::Mat_<std::uint8_t>& mks_cl,
        const cv::Mat_<std::uint8_t>& masks_cl,
        float cell_r_um,
        float cell_c_um,
        float border_um,
        int rows, 
        int cols,
        std::uint32_t invl_x_cl, 
        std::uint32_t invl_y_cl,
        float um2px_r
    ) const {
        auto [candi_mk_pats_px, candi_mk_pats_px_mask, invl_x_px, invl_y_px, border_px] = 
            to_px_domain({mks_cl}, {masks_cl}, cell_r_um, cell_c_um, border_um, 
                invl_x_cl, invl_y_cl, um2px_r
            );
        marker::Layout mk_layout;
        mk_layout.set_reg_mat_dist(
            rows, cols, {0,0},
            invl_x_cl, invl_y_cl,
            invl_x_px, invl_y_px,
            border_px
        );
        mk_layout.set_single_mk_pat(
            {mks_cl},
            candi_mk_pats_px,
            {masks_cl},
            candi_mk_pats_px_mask
        );
        return mk_layout;
    }
    /**
     * @brief Set marker layout by given new marker and um to pixel rate.
     * 
     * @param mks_cl        New marker pattern in cell level.
     * @param masks_cl      New marker mask in cell level.
     * @param cell_r_um     Cell height in micron.
     * @param cell_c_um     Cell width in micron.
     * @param border_um     Border size between cell in micron.
     * @param invl_x_cl     Marker interval along X direction in cell level. 
     * @param invl_y_cl     Marker interval along Y direction in cell level. 
     * @param um2px_r       Micron to pixel rate.
     * @param mk_layout     Existing marker layout.
     * @return auto         Deduced, void.
     */
    auto operator()(
        const std::vector<cv::Mat_<std::uint8_t>>& mks_cl,
        const std::vector<cv::Mat_<std::uint8_t>>& masks_cl,
        float cell_r_um,
        float cell_c_um,
        float border_um,
        std::uint32_t invl_x_cl, 
        std::uint32_t invl_y_cl,
        float um2px_r,
        marker::Layout& mk_layout
    ) const {
        auto [candi_mk_pats_px, candi_mk_pats_px_mask, invl_x_px, invl_y_px, border_px] = 
            to_px_domain(mks_cl, masks_cl, cell_r_um, cell_c_um, border_um, 
                invl_x_cl, invl_y_cl, um2px_r
            );
        mk_layout.reset_mk_pat(
            candi_mk_pats_px,
            candi_mk_pats_px_mask,
            invl_x_px, invl_y_px,
            border_px
        );
    }
    /**
     * @brief Reset marker layout by given new micron to pixel rate 
     *        and other micron parameter.
     * 
     * @param mk_layout     Existing marker layout. 
     * @param cell_r_um     Cell height in micron.
     * @param cell_c_um     Cell width in micron.
     * @param border_um     Border size between cell in micron.
     * @param um2px_r       Micron to pixel rate.
     * @return auto         Deduced, void. 
     */
    auto operator()(
        marker::Layout& mk_layout, 
        float cell_r_um,
        float cell_c_um,
        float border_um,
        float um2px_r
    ) const {
        auto mk_des = mk_layout.get_single_pat_marker_des();
        auto& mks = mk_des.get_candi_mks(MatUnit::CELL);
        auto& masks = mk_des.get_candi_mks_mask(MatUnit::CELL);
        operator()(
            mks, masks, 
            cell_r_um, cell_c_um,
            border_um,
            mk_layout.mk_invl_x_cl,
            mk_layout.mk_invl_y_cl,
            um2px_r,
            mk_layout
        );
    }
private:
    chipimgproc::marker::TxtToImg txt_to_img_;
} 
/**
 * @brief Global functor with MakeSinglePatternRegMatLayout type.
 * 
 */
make_single_pattern_reg_mat_layout;



}}
