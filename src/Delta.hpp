#ifndef DELTA_HPP
#define DELTA_HPP

#include <iostream>

struct Delta {
    int delta[4] = {0};
    static constexpr int MAX_DELTA = 10 * 10 * 10 * 10;

    inline int id() const {
        int id = 0;
        for (int i = 0; i < 4; ++i)
            id = id * 10 + delta[i];
        return id;
    }

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);
    bool operator<(const Delta& o) const;
    bool operator>(const Delta& o) const;

    friend std::ostream& operator<<(std::ostream& out, const Delta& o) {
        return out << "{" << o.delta[0] << "," << o.delta[1] << ","
                   << o.delta[2] << "," << o.delta[3] << "}";
    }

    friend Delta operator+(const Delta& d1, const Delta& d2) {
        Delta res;
        for (int i = 0; i < 4; ++i)
            res[i] = d1[i] + d2[i];
        return res;
    }

    static Delta decode(int id);
};

#endif /* DELTA_HPP */
