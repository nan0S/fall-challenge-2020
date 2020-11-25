
#ifndef COMMON_HPP
#define COMMON_HPP

#include <iostream>
#include <chrono>

using eval_t = float;

constexpr int INF = 1e9;

#define low(x) ((x) & (-(x))) // lowest bit
#define bits(x) (31 - __builtin_clz(x)) // floor(log2(x))

#if defined(LOCAL) && !defined(NDEBUG)
#define DEBUG
#endif

#ifdef DEBUG

inline void __debug(const char* s, const char*) {
    std::cerr << s << ": ";
}

template<typename T>
void __debug(const char* s, const T& x) {
    std::cerr << s << ": " << x << " ";
}

template<typename T, typename... Args>
void __debug(const char* s, const T& x, const Args&... rest) {
    int bracket = 0;
    char c;
    while ((c = *s) != ',' || bracket)
    {
        std::cerr << *s++;
        switch (c)
        {
            case '(':
            case '{':
            case '[':
                ++bracket;
                break;
            case ')':
            case '}':
            case ']':
                --bracket;
        }
    }
    std::cerr << ": ";
    std::cerr << x << ",";
    __debug(s + 1, rest...);

}

template<typename... Args>
void _debug(const char* s, const Args&... rest) {
    __debug(s, rest...);
    std::cerr << std::endl;
}

#define debug(...) _debug(#__VA_ARGS__, __VA_ARGS__)
#else
#define debug(...) 13
#endif

class Timer {
public:
    Timer(float timeLimit);
    bool isTimeLeft() const;

private:
    float timeLimit;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};

#endif /* COMMON_HPP */
