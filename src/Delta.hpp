#ifndef DELTA_HPP
#define DELTA_HPP

#include "Common.hpp"

#include <iostream>

using delta_t = int;

struct Delta {
    delta_t delta[4] = {0};

    static constexpr int LIMIT = 11;

    bool canApply(const Delta& d) const;
    eval_t eval() const;

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);

    friend std::istream& operator>>(std::istream& in, Delta& d);
    friend std::ostream& operator<<(std::ostream& out, const Delta& o);
    friend Delta operator+(const Delta& d1, const Delta& d2);

    static Delta decode(int id);
};

#endif /* DELTA_HPP */
