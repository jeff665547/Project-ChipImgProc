#pragma once
#include <set>
#include <Nucleona/language.hpp>
namespace chipimgproc {

template<class T, class... Args>
struct FixedCapacitySet 
: public std::set<T, Args...> {
    using Base = std::set<T, Args...>;

    FixedCapacitySet(std::size_t n, Args&&... args)
    : Base(FWD(args)...)
    , max_size_ (n)
    {}
    template<class... Params>
    decltype(auto) emplace(Params&&... obj) {
        typename Base::key_type key(FWD(obj)...);
        if(Base::size() < max_size_) {
            last_emplace_ = Base::emplace(std::move(key));
            return last_emplace_;
        }
        else if(Base::key_comp()(*Base::begin(), key)) {
            Base::erase(Base::begin());
            last_emplace_ = Base::emplace(std::move(key));
            return last_emplace_;
        } else {
            return last_emplace_;
        }
    }
private:
    using Base::emplace_hint;
    using Base::insert;
    std::size_t max_size_;
    std::pair<typename Base::iterator, bool> last_emplace_;
};
template<class T, class FUNC>
auto make_fixed_capacity_set(std::size_t n, FUNC&& comp) {
    return FixedCapacitySet<T, std::decay_t<FUNC>>(n, FWD(comp));
}
}