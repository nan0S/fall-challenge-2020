#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "Delta.hpp"
#include "Action.hpp"

#include <vector>

class Battle {
public:
    static void start();

private:
    static void readData();
    static const Action* pickAction();
    static const Action* getOrder();
    static float eval(const Delta& v, int d);

private:
    static Witch player;
    static Witch opponent;

    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;
    
    static Rest rest;

    static int dist[];
    static int from[];
    static int act[];

    static int roundNumber;
};

#endif /* BATTLE_HPP */
