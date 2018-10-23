#pragma once
#include <vector>
#include "utils.hpp"

namespace chipimgproc::aruco {

class Dictionary : public std::vector<std::uint64_t> {
  public:
    Dictionary(const std::int32_t coding_bits, const std::int32_t maxcor_bits)
      : coding_bits_(coding_bits)
      , maxcor_bits_(maxcor_bits) {
    }
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

    auto coding_bits(void) const {
        return coding_bits_;
    }

    auto maxcor_bits(void) const {
        return maxcor_bits_;
    }

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