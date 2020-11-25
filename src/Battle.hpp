#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "Delta.hpp"
#include "Action.hpp"
#include "Common.hpp"

#include <vector>

struct State {
    Witch player;

    std::vector<Spell> spells;
    std::vector<Order> orders;
    std::vector<Recipe> recipes;

    const Action* firstAction = nullptr;
    int ordersDone = 0;

    std::vector<State> getNeighbors() const;
    eval_t eval() const;

    bool operator<(const State& s) const;
    friend std::ostream& operator<<(std::ostream& out, const State& s);
};

class Battle {
public:
    static void start();

public:
    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;
    static Rest rest;

    static std::vector<Spell> tSpells;

    static int playerOrdersDone;
    static int enemyOrdersDone;

private:
    static void readData();
    #ifdef DEBUG
    static void writeData();
    #endif
    static const Action* pickAction();
    static const Action* chooseRecipe();
    static const Action* search();
    static State getInitialState();

private:
    static Witch player;
    static Witch opponent;

    static int roundNumber;

    static constexpr int beamDepth = 5;
    static constexpr int beamWidth = 5;
    static constexpr int recipeConsider = 2;
};

#endif /* BATTLE_HPP */
