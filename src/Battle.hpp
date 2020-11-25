#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "Delta.hpp"
#include "Action.hpp"
#include "Common.hpp"

#include <vector>
#include <array>

struct State;

class Battle {
public:
    static void start();

public:
    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;
    static std::vector<Spell> tSpells;
    static Rest rest;

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
    static constexpr int beamWidth = 600;
};

struct State {
    const Action* firstAction = nullptr;
    Witch player;
    int castableSpellsMask = 0;
    int ordersUndoneMask = 0;
    float gamma = 1.f;

    std::vector<Delta> history;

    static constexpr int MAX_NEIGHBORS = 30;
    static constexpr float DECAY = 0.95f;

    int getNeighbors(std::array<State, MAX_NEIGHBORS>& neighbors) const;
    inline eval_t eval() const;
    inline int getOrdersDone() const;
    inline bool isCastable(const int& i) const;
    inline bool isOrderDone(const int& i) const;

    bool operator<(const State& s) const;
    friend std::ostream& operator<<(std::ostream& out, const State& s);
};

eval_t State::eval() const {
    eval_t value = player.eval();
    // int ordersDone = getOrdersDone() * 3 + 10;
    // value += 5 * ordersDone * ordersDone;
    return value;
}

int State::getOrdersDone() const {
    return Battle::orders.size() - __builtin_popcount(ordersUndoneMask);
}

bool State::isCastable(const int& i) const {
    return castableSpellsMask & 1 << i;
}

bool State::isOrderDone(const int& i) const {
    return !(ordersUndoneMask & 1 << i);
}

#endif /* BATTLE_HPP */
