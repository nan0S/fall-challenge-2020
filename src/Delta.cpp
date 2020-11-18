#include "Delta.hpp"

#include <cassert>

int& Delta::operator[](const int& idx) {
    return delta[idx];
}

const int& Delta::operator[](const int& idx) const {
    return delta[idx];
}

Delta& Delta::operator+=(const Delta& o) {
    for (int i = 0; i < 4; ++i)
        delta[i] += o[i];
    return *this;
}

bool Delta::operator<(const Delta& o) const {
    int s = 0;
    for (int i = 0; i < 4; ++i) {
        int diff = delta[i] + o[i];
        if (diff < 0)
            return true;
        s += diff;
    }
    return s > 10; 
}

bool Delta::operator>(const Delta& o) const {
    return o < *this;
}

Delta Delta::decode(int id) {
    Delta res;
    for (int i = 3; i >= 0; --i) {
        res[i] = id % 10;
        id /= 10;
    }
    assert(id == 0);
    return res;
}
