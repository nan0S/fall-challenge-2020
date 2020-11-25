#ifndef DELTA_HPP
#define DELTA_HPP

#include "Common.hpp"

#include <iostream>

using delta_t = int;

struct Delta {
    delta_t delta[4] = {0};

    bool canApply(const Delta& d) const;
    inline eval_t eval() const;

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);

    friend std::istream& operator>>(std::istream& in, Delta& d);
    friend std::ostream& operator<<(std::ostream& out, const Delta& o);
    friend Delta operator+(const Delta& d1, const Delta& d2);
};

eval_t Delta::eval() const {
    eval_t value = 0;
    for (int i = 0; i < 4; ++i)
        value += delta[i] * (i + 1);
    return value;
}

#endif /* DELTA_HPP */
