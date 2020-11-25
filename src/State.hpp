#ifndef STATE_HPP
#define STATE_HPP

#include "Action.hpp"

#include <vector>

struct State {
    Witch player;

    std::vector<Spell> spells;
    std::vector<Order> orders;
    std::vector<Recipe> recipes;

    const Action* firstAction = nullptr;
    int ordersDone = 0;
    static constexpr eval_t DECAY = 0.9f;
    eval_t decay = 1;

    std::vector<State> getNeighbors() const;
    eval_t eval() const;

    bool operator<(const State& s) const;
    friend std::ostream& operator<<(std::ostream& out, const State& s);
};

#endif /* STATE_HPP */
