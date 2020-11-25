#include "State.hpp"


bool State::operator<(const State& s) const {
    return eval() < s.eval();
}

eval_t State::eval() const {
    eval_t value = player.eval();
    return value;
}

std::ostream& operator<<(std::ostream& out, const State& s) {
    out << s.player << "\n";
    out << "ordersDone: " << s.ordersDone << "\n";
    out << "Spells:\n";
    for (const auto& spell : s.spells)
        out << "\t" << spell << "\n";
    out << "Orders:\n";
    for (const auto& order : s.orders)
        out << "\t" << order << "\n";
    out << "Recipes:\n";
    for (const auto& recipe : s.recipes)
        out << "\t" << recipe << "\n";
    return out;
}

std::vector<State> State::getNeighbors() const {
    std::vector<State> neighbors;

    for (int i = 0; i < int(orders.size()); ++i) {
        const auto& order = orders[i];
        if (player.inv.canApply(order.delta)) {
            State neighbor = *this;
            neighbor.player.inv += order.delta;
            neighbor.player.score += order.price * decay;
            neighbor.ordersDone++;
            neighbor.decay *= DECAY;
            if (firstAction == nullptr)
                neighbor.firstAction = &Battle::orders[i];
            neighbors.push_back(neighbor);
        }
    }

    for (int i = 0; i < int(spells.size()); ++i) {
        const auto& s = spells[i];
        if (s.castable) {
            auto spell = s;
            for (int times = 0; times < s.maxTimes; ++times) {
                if (player.inv.canApply(spell.delta)) {
                    State neighbor = *this;
                    neighbor.player.inv += spell.delta;
                    neighbor.spells[i].castable = false;
                    if (firstAction == nullptr) {
                        Battle::tSpells.push_back(Battle::spells[i]);
                        Battle::tSpells.back().curTimes = times + 1;
                        neighbor.firstAction = &Battle::tSpells.back();
                    }
                    neighbors.push_back(neighbor);
                }
                else
                    break;
                spell.delta += s.delta;
            } 
        }
    }

    // TODO: recipes

    auto neighbor = *this;
    for (auto& spell : neighbor.spells)
        spell.castable = true;
    if (firstAction == nullptr)
        neighbor.firstAction = &Battle::rest;
    neighbors.push_back(neighbor);

    return neighbors;
}
