#include "Battle.hpp"
#include "Common.hpp"

#include <queue>
#include <cassert>
#include <set>
#include <vector>

Witch Battle::player;
Witch Battle::opponent;

std::vector<Spell> Battle::spells;
std::vector<Order> Battle::orders;
std::vector<Recipe> Battle::recipes;

Rest Battle::rest;

int Battle::distance[Delta::MAX_DELTA];
int Battle::from[Delta::MAX_DELTA];
int Battle::act[Delta::MAX_DELTA];

int Battle::roundNumber = 0;

void Battle::start() {
    orders.reserve(100);
    spells.reserve(100);
    recipes.reserve(100);

    while (true) {
        readData();
        pickAction()->print();

        orders.clear();
        spells.clear();
        recipes.clear();
        
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
}

const Action* Battle::pickAction() {
    if (roundNumber < 10)
        return &recipes.front();

    auto orderAction = getDoableOrder();
    if (orderAction)
        return orderAction;

    auto serchAction = search();
    if (serchAction)
        return serchAction;

    return &rest;
}

const Action* Battle::getDoableOrder() {
    const Action* bestAction = nullptr;
    int bestPrice = -1;

    for (const auto& action : orders)
        if (!(player.inv < action.delta) && bestPrice < action.price) {
            bestPrice = action.price;
            bestAction = &action;
        }

    return bestAction;
}

const Action* Battle::search() {
    std::fill(distance, distance + Delta::MAX_DELTA, -1);
    distance[player.inv.id()] = 0;
    std::queue<Delta> q;
    q.push(player.inv);

    float bestValue = -1.f;
    Delta bestState;

    while (!q.empty()) {
        auto v = q.front();
        q.pop();

        int vid = v.id();
        int d = distance[vid];

        float value = eval(v, d);
        if (value > bestValue) {
            bestValue = value;
            bestState = v;
        }

        for (int i = 0; i < int(spells.size()); ++i) {
            const auto& action = spells[i];
            if (!(v < action.delta)) {
                Delta s = v + action.delta;
                int id = s.id();
                if (distance[id] == -1) {
                    distance[id] = d + 1;
                    from[id] = vid;
                    act[id] = i;
                    q.push(s);
                }
            }
        }
    }

    debug(bestState, bestValue);

    int id = bestState.id();
    int d = distance[id];
    assert(d >= 1);
    std::set<int> actions;

    while (d > 0) {
        actions.insert(act[id]);
        id = from[id];
        d = distance[id];
    }

    for (int idx : actions)
        debug(spells[idx]);

    assert(!actions.empty());
    for (int idx : actions) {
        const auto& action = spells[idx];
        if (action.castable && !(player.inv < action.delta))
            return &action;
    }

    return nullptr;
}

float Battle::eval(const Delta& v, int d) {
    int maxPrice = 0;

    for (const auto& action : orders)
        if (!(v < action.delta) && maxPrice < action.price)
            maxPrice = action.price;

    return d == 0 ? 0 : float(maxPrice) / d;
}
