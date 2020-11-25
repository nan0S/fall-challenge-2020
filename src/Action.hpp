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

    Order(const int& id, const Delta& delta, const int& price);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Order& o);
};

struct Spell : public Action {
    std::array<Delta, 10> repeatedDeltas;
    int maxTimes = 1;
    int curTimes = 1;
    bool castable;
    bool repeatable;

    Spell(const int& id, const Delta& delta,
        const bool& castable, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Spell& s);
};

struct Recipe : public Action {
    int tomeIndex;
    int taxCount;
    bool repeatable;

    Recipe(const int& id, const Delta& delta,
        const int &tomeIndex, const int& taxCount, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& r);
};

struct Rest : public Action {
	Rest();
    void print() const override;
};

struct Witch {
    Delta inv;
    float score;

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
