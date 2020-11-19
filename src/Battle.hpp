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
    #ifdef DEBUG
    static void writeData();
    #endif
    static const Action* pickAction();
    static const Action* getDoableOrder();
    static const Action* search();
    static float eval(const Delta& v, int dist);

private:
    static Witch player;
    static Witch opponent;

    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;

    static Rest rest;

    static int distance[];
    static int from[];
    static int fromidx[];
    static int fromtimes[];

    struct Info {
        int id;
        int dist;
    };
    static std::vector<Info> orderCost;

    static int roundNumber;
    static int enemyOrdersDone;
};

#endif /* BATTLE_HPP */
