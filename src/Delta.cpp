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
    return s > LIMIT; 
}

bool Delta::operator>(const Delta& o) const {
    return o < *this;
}

Delta Delta::decode(int id) {
    Delta res;
    for (int i = 3; i >= 0; --i) {
        res[i] = id % LIMIT;
        id /= LIMIT;
    }
    assert(id == 0);
    return res;
}

std::istream& operator>>(std::istream& in, Delta& d) {
    return in >> d.delta[0] >> d.delta[1]
              >> d.delta[2] >> d.delta[3];
}


std::ostream& operator<<(std::ostream& out, const Delta& o) {
    return out << "{" << o.delta[0] << "," << o.delta[1] << ","
               << o.delta[2] << "," << o.delta[3] << "}";
}

Delta operator+(const Delta& d1, const Delta& d2) {
    Delta res;
    for (int i = 0; i < 4; ++i)
        res[i] = d1[i] + d2[i];
    return res;
}
