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

struct MatUnit {
    enum State {
        PX, CELL
    };
    MatUnit(const State& s)
    : s_ (s)
    {}
    std::string to_string() const {
        switch(s_) {
            case PX     : return "pixel" ;
            case CELL   : return "cell"  ;
            default: 
                throw std::runtime_error("MatUnit::to_string, unsupport type");
        }
    }
    operator std::string() const {
        return to_string();
    }
    operator const State&() const {
        return s_;
    }
private:
    State s_;
};

}