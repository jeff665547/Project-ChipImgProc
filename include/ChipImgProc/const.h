/**
 * @file    const.h
 * @author  Chia-Hua Chang (johnidfet@centrilliontech.com.tw)
 * @brief   Several global variable or enum type definition
 * 
 */
#pragma once
#include <string>
namespace chipimgproc{ 

struct OriginPos {
    enum State{
        LT, LB
    };
    operator std::string() {
        switch(s_) {
            case LT: return "LT";
            case LB: return "LB";
        }
    }
    operator State& () {
        return s_;
    }
    operator const State& () const {
        return s_;
    }
private:
    State s_;
};

struct XDirect {
    enum State {
        H, V
    };

    operator std::string() {
        switch(s_) {
            case H: return "_";
            case V: return "|";
        }
    }
    operator State& () {
        return s_;
    }
    operator const State& () const {
        return s_;
    }
private:
    State s_;
};

/**
 * @brief The unit presented in image matrix element, can be PX(pixel) or CELL(cell).
 * 
 */
struct MatUnit {
    /**
     * @brief internal enum data type 
     * 
     */
    enum State {
        PX, CELL
    };
    /**
     * @brief Construct a new Mat Unit object
     * 
     * @param s PX/CELL
     */
    MatUnit(const State& s)
    : s_ (s)
    {}
    /**
     * @brief Convert MatUnit to string.
     * @details PX is "pixel", CELL is "cell".
     * @return std::string 
     */
    std::string to_string() const {
        switch(s_) {
            case PX     : return "pixel" ;
            case CELL   : return "cell"  ;
            default: 
                throw std::runtime_error("MatUnit::to_string, unsupported type");
        }
    }
    /**
     * @brief String cast operator of MatUnit, equivlent to_string
     * 
     * @return std::string 
     */
    operator std::string() const {
        return to_string();
    }
    /**
     * @brief Internal data cast
     * 
     * @return const State& 
     */
    operator const State&() const {
        return s_;
    }
private:
    State s_;
};

}