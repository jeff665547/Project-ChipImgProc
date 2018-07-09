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

}