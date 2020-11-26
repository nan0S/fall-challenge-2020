#ifndef ACTION_HPP
#define ACTION_HPP

#include "Delta.hpp"

#include <array>

struct Action {
    Delta delta;
    int id;

    Action() = default;
    Action(const int& id, const Delta& delta);

    virtual void print() const = 0;
    virtual ~Action() = default;
};

struct Order : public Action {
    int price;

    Order() = default;
    Order(const int& id, const Delta& delta, const int& price);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Order& o);
};

struct Recipe : public Action {
    int tomeIndex;
    int taxCount;
    bool repeatable;

    Recipe() = default;
    Recipe(const int& id, const Delta& delta,
        const int &tomeIndex, const int& taxCount, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& r);
};

struct Spell : public Action {
    bool castable;
    bool repeatable;
    int maxTimes = 1;
    int curTimes = 1;

    static constexpr int MAX_REPEATED_DELTA = 10;
    std::array<Delta, MAX_REPEATED_DELTA> repeatedDeltas;

    Spell() = default;
    Spell(const int& id, const Delta& delta,
        const bool& castable, const bool& repeatable);
    Spell(const Recipe& recipe);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Spell& s);
};

struct Rest : public Action {
	Rest() = default;
    void print() const override;
};

struct Witch {
    Delta inv;
    int score;

    inline eval_t eval() const;

    friend std::istream& operator>>(std::istream& in, Witch& w);
    friend std::ostream& operator<<(std::ostream& out, const Witch& w);
};

eval_t Witch::eval() const {
    eval_t value = score;
    value += inv.eval();
    return value;
}

#endif /* ACTION_HPP */
