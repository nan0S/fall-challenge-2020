#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "Delta.hpp"
#include <vector>

struct Order {
    int id;
    Delta delta;
    int price;

    friend std::ostream& operator<<(std::ostream& out, const Order& order) {
        return out << "ORDER: id=" << order.id 
            << ", delta=" << order.delta << ", "
            << "price=" << order.price;
    }
};

struct Spell {
    int id;
    Delta delta;
    bool castable;

    friend std::ostream& operator<<(std::ostream& out, const Spell& spell) {
        return out << "SPELL: id=" << spell.id 
            << ", delta={" << spell.delta << ", "
            << "castable=" << spell.castable;
    }
};

struct Recipe {
    int id;
    Delta delta;
    int tomeIndex;
    int taxCount;
    bool repeatable;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& recipe) {
        return out << "RECIPE: id=" << recipe.id 
            << ", delta={" << recipe.delta << ", "
            << "tomeIndex=" << recipe.tomeIndex << ", "
            << "taxCount=" << recipe.taxCount << ", "
            << "repeatable=" << recipe.repeatable;
    }
};

class Battle {
public:
    static void start();

private:
    static float eval(const Delta& v, int d);
    static int getOrder();
    static void pickAction();

private:
    static Delta mInv, oInv;
    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;

    static int dist[];
    static int from[];
    static int act[];
};

#endif /* BATTLE_HPP */
