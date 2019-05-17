/**
 *  @file    ChipImgProc/aruco/dictionary.hpp
 *  @author  Chia-Hua Chang, Alex Lee
 *  @brief   ArUco marker dictionary
 */
#pragma once
#include <vector>
#include "utils.hpp"

namespace chipimgproc::aruco {
/**
 *  @brief  Encoding ArUco marker into 64bit integer and stored as a vector.
 *  @details Here shows an example
 *  @snippet ChipImgProc/aruco_test.cpp usage
 */
class Dictionary : public std::vector<std::uint64_t> {
  public:
    /**
     *  @brief  Dictionary default constructor.
     */
    Dictionary() = default;
    /**
     *  @brief  Dictionary constructor.
     *  @param  coding_bits  The value defines the size of an ArUco marker
     *  @param  maxcor_bits  The maximum number of bits for marker correction
     */
    Dictionary(const std::int32_t coding_bits, const std::int32_t maxcor_bits)
      : coding_bits_(coding_bits)
      , maxcor_bits_(maxcor_bits) {
    }
    /**
     *  @brief  Generate dictionary from json.
     *  @param  j_dict  ArUco dictionary specified in json format. 
     *                  For example:
     *  @code
     *  {
     *    "coding_bits": 6
     *  , "maxcor_bits": 5
     *  , "bitmap_list": {
     *        "0": [
     *            0, 0, 0, 1, 1, 1
     *          , 1, 0, 0, 0, 1, 1
     *          , 1, 1, 0, 1, 1, 1
     *          , 0, 1, 1, 0, 0, 0
     *          , 0, 0, 1, 0, 1, 0
     *          , 1, 0, 0, 1, 1, 0
     *        ]
     *      , "1": [
     *            0, 0, 0, 0, 1, 1
     *          , 1, 0, 1, 1, 1, 1
     *          , 1, 0, 1, 1, 1, 0
     *          , 1, 0, 0, 0, 1, 1
     *          , 1, 0, 0, 0, 1, 0
     *          , 0, 1, 0, 0, 0, 1
     *        ]
     *      , ...
     *    }
     *  }
     *  @endcode
     *  @return  A Dictionary object
     */
    static Dictionary from_json( const nlohmann::json& j_dict ) {
        Dictionary dict(
            j_dict["coding_bits"].get<std::int32_t>(),
            j_dict["maxcor_bits"].get<std::int32_t>()
        );
        dict.resize(j_dict["bitmap_list"].size());
        auto&& list = j_dict["bitmap_list"];
        for (auto item = list.begin(); item != list.end(); ++item) {
            auto&& bitmap = item.value();
            uint64_t code = 0; // store with litten-endian
            for (auto offset = 0; offset != bitmap.size(); ++offset)
                code |= bitmap[offset].get<uint64_t>() << offset;
            dict[std::stoi(item.key())] = code;
        }
        return dict;
    }

    /**
     * @brief   Get the max number of coding bits
     * @return  The number of coding bits
     */
    auto coding_bits(void) const {
        return coding_bits_;
    }

    /**
     * @brief   Get the max number of correction bits
     * @return  The max number of correction bits
     */
    auto maxcor_bits(void) const {
        return maxcor_bits_;
    }

    /**
     * @brief   Compute the max number of correction bits based on user specified ID list.
     *          Suppose there are m markers. The time complexity would be O(m^2).
     * 
     * @param   ids   A list of marker IDs defined in the dictionary
     * @return  max   Hamming distance of specified id list
     */
    auto compute_maxcor_bits(const std::vector<std::int32_t>& ids) const {
        std::int32_t min_distance = coding_bits_ * coding_bits_;
        for (auto j = 1; j < ids.size(); ++j) {
            for (auto i = 0; i != j; ++i) {
                if (ids[i] == ids[j])
                    throw std::invalid_argument("duplicated indices detected\n");
                auto distance = Utils::bit_count(this->at(ids[i]) ^ this->at(ids[j]));
                if (min_distance > distance)
                    min_distance = distance;
            }
        }
        return min_distance / 2;
    }
  
    /**
     *  @brief       Identify the query code from a given candidate marker ID list.
     *               This function will output the closest candidate 
     *               if the hamming distance between the query and closest one
     *               is less than the max number of correction bits.
     * 
     *  @param       query        The query code.
     *  @param[out]  index        The closest marker ID in candidate list.
     *  @param       candidates   A list of candidate marker IDs.
     *  @param       maxcor_bits  The max number of correction bits.
     *  @return      True if the query code is approximately in the candidate list.
     *               False if the search fails.
     */
    bool identify(
        const std::uint64_t query
      , std::int32_t& index
      , const std::vector<std::int32_t>& candidates
      , const std::int32_t maxcor_bits = -1
    ) const {
        index = -1;
        std::int32_t distance = (maxcor_bits != -1) ? maxcor_bits : maxcor_bits_ ;
        for (auto&& i : candidates) {
            auto d = Utils::bit_count(query ^ this->at(i));
            if (distance > d) {
                distance = d, index = i;
                if (distance == 0)
                    break;
            }
        }
        return index != -1;
    }

    /**
     *  @brief       Identify whether the query code is approximately belong to the dictionary, and
     *               output the index of the closest one.
     * 
     *  @param       query  The query code.
     *  @param[out]  index  The index of the closest code in dictionary.
     *                      Set the value to -1 if the search fails.
     *  @return      True if the query code is approximately in the dictionary.
     *               False if the search fails.
     */
    bool identify(
        const std::uint64_t query
      , std::int32_t& index
    ) const {
        index = -1;
        std::int32_t distance = maxcor_bits_;
        for (auto i = 0u; i != this->size(); ++i) {
            auto d = Utils::bit_count(query ^ (*this)[i]);
            if (distance > d) {
                distance = d, index = i;
                if (distance == 0)
                    break;
            }
        }
        return index != -1;
    }

  private:
    std::int32_t coding_bits_;
    std::int32_t maxcor_bits_;
};

} // namespace