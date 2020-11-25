#include "Battle.hpp"
#include "Options.hpp"

#include <queue>
#include <cassert>
#include <set>
#include <vector>
#include <algorithm>
#include <math.h>

bool State::operator<(const State& s) const {
    return eval() < s.eval();
}

eval_t State::eval() const {
    eval_t value = player.eval();
    value += 5 * ordersDone * ordersDone;
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
            neighbor.player.score += order.price;
            neighbor.ordersDone++;
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

    auto neighbor = *this;
    for (auto& spell : neighbor.spells)
        spell.castable = true;
    if (firstAction == nullptr)
        neighbor.firstAction = &Battle::rest;
    neighbors.push_back(neighbor);

    return neighbors;
}

Witch Battle::player;
Witch Battle::opponent;

std::vector<Spell> Battle::spells;
std::vector<Order> Battle::orders;
std::vector<Recipe> Battle::recipes;

Rest Battle::rest;

std::vector<Spell> Battle::tSpells;

int Battle::roundNumber = 0;
int Battle::enemyOrdersDone = 0;
int Battle::playerOrdersDone = 0;

void Battle::start() {
    orders.reserve(100);
    spells.reserve(100);
    recipes.reserve(100);
    tSpells.reserve(100);

    while (true) {
        readData();
        #ifdef DEBUG
        // writeData();
        #endif
        pickAction()->print();

        orders.clear();
        spells.clear();
        recipes.clear();
        tSpells.clear();
        
        ++roundNumber;
    }
}

void Battle::readData() {
    int actionCount;
    std::cin >> actionCount;

    while (actionCount--) {
        int actionId;
        std::string actionStr;
        std::cin >> actionId >> actionStr;

        Delta delta;
        std::cin >> delta;

        int price, tomeIndex, taxCount;
        bool castable, repeatable;
        std::cin >> price >> tomeIndex >> taxCount;
        std::cin >> castable >> repeatable;

        if (actionStr == "BREW")
            orders.emplace_back(actionId, delta, price);
        else if (actionStr == "CAST")
            spells.emplace_back(actionId, delta, castable, repeatable);
        else if (actionStr == "LEARN")
            recipes.emplace_back(actionId, delta, tomeIndex, taxCount, repeatable);
        else
            assert(actionStr == "OPPONENT_CAST");
    }     

    std::cin >> player;
    std::cin >> opponent;

    static int lastPlayerScore = 0;
    if (player.score != lastPlayerScore) {
        lastPlayerScore = player.score;
        ++playerOrdersDone;
    }

    static int lastEnemyScore = 0;
    if (opponent.score != lastEnemyScore) {
        lastEnemyScore = opponent.score;
        ++Options::enemyOrdersDone;
    }
}

#ifdef DEBUG
void Battle::writeData() {
    for (const auto& order : orders)
        debug(order);
    for (const auto& spell : spells)
        debug(spell);
    for (const auto& recipe : recipes)
        debug(recipe);    
}
#endif

const Action* Battle::pickAction() {
    if (roundNumber < 10)
        return chooseRecipe();
    return search();
}

const Action* Battle::chooseRecipe() {
    return &recipes.front();
}

const Action* Battle::search() {
    State initialState = getInitialState();
    std::priority_queue<State> q, layer;
    q.push(initialState);

    for (int depth = 0; depth < beamDepth; ++depth) {
        assert(!q.empty());

        for (int i = 0; i < beamWidth; ++i) {
            const auto state = q.top();
            q.pop();

            auto neighbors = state.getNeighbors();
            for (const auto& neighbor : neighbors)
                layer.push(neighbor);

            if (i != beamWidth - 1 && q.empty()) {
                debug("Exiting loop early - not enough states!");
                break;
            }
        }

        while (!q.empty())
            q.pop();
        q.swap(layer);
    }

    assert(!q.empty());
    const auto& finalState = q.top();
    debug(finalState);    
    assert(finalState.firstAction != nullptr);

    return finalState.firstAction;
}

State Battle::getInitialState() {
    State initialState;
    initialState.player = player;
    initialState.player.score = 0;
    initialState.ordersDone = playerOrdersDone;
    initialState.spells = spells;
    initialState.orders = orders;
    initialState.recipes = recipes;
    return initialState;
}
