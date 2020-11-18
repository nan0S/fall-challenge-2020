#ifndef DELTA_HPP
#define DELTA_HPP

#include <iostream>

using delta_t = int;

struct Delta {
    delta_t delta[4] = {0};

    static constexpr int LIMIT = 10;
    static constexpr int MAX_DELTA = LIMIT * LIMIT * LIMIT * LIMIT;

    inline int id() const {
        int id = 0;
        for (int i = 0; i < 4; ++i)
            id = id * LIMIT + delta[i];
        return id;
    }

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);
    bool operator<(const Delta& o) const;
    bool operator>(const Delta& o) const;

    friend std::istream& operator>>(std::istream& in, Delta& d);
    friend std::ostream& operator<<(std::ostream& out, const Delta& o);
    friend Delta operator+(const Delta& d1, const Delta& d2);

    static Delta decode(int id);
};

#endif /* DELTA_HPP */
