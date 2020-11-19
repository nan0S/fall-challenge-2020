#ifndef ACTION_HPP
#define ACTION_HPP

#include "Delta.hpp"

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

    friend std::ostream& operator<<(std::ostream& out, const Order& order);
};

struct Spell : public Action {
    int maxTimes = 1;
    int times = 1;
    bool castable;
    bool repeatable;

    Spell(const int& id, const Delta& delta,
        const bool& castable, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Spell& spell);
};

struct Recipe : public Action {
    int tomeIndex;
    int taxCount;
    bool repeatable;

    Recipe(const int& id, const Delta& delta,
        const int &tomeIndex, const int& taxCount, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& recipe);
};

struct Rest : public Action {
	Rest();
    void print() const override;
};

struct Witch {
    Delta inv;
    int score;

    friend std::istream& operator>>(std::istream& in, Witch& witch);
};

#endif /* ACTION_HPP */
