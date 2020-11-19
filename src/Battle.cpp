#include "Battle.hpp"
#include "Common.hpp"

#include <queue>
#include <cassert>
#include <set>
#include <vector>
#include <algorithm>

Witch Battle::player;
Witch Battle::opponent;

std::vector<Spell> Battle::spells;
std::vector<Order> Battle::orders;
std::vector<Recipe> Battle::recipes;

Rest Battle::rest;

int Battle::distance[Delta::MAX_DELTA];
int Battle::from[Delta::MAX_DELTA];
int Battle::fromidx[Delta::MAX_DELTA];
int Battle::fromtimes[Delta::MAX_DELTA];

std::vector<Battle::Info> Battle::orderCost;

int Battle::roundNumber = 0;
int Battle::enemyOrdersDone = 0;

void Battle::start() {
    orders.reserve(100);
    spells.reserve(100);
    recipes.reserve(100);

    while (true) {
        readData();
        #ifdef DEBUG
        writeData();
        #endif
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

    static int lastEnemyScore = 0;
    if (opponent.score != lastEnemyScore) {
        lastEnemyScore = opponent.score;
        ++enemyOrdersDone;
        debug(enemyOrdersDone);
    }
}

void Battle::writeData() {
    for (const auto& order : orders)
        debug(order);
    for (const auto& spell : spells)
        debug(spell);
    for (const auto& recipe : recipes)
        debug(recipe);    
}

const Action* Battle::pickAction() {
    if (roundNumber < 7)
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
    // initialization
    std::fill(distance, distance + Delta::MAX_DELTA, -1);
    orderCost.assign(orders.size(), {-1, INF});

    distance[player.inv.id()] = 0;
    std::queue<Delta> q;
    q.push(player.inv);

    // useful variables
    const int spellCount = int(spells.size());

    // bfs
    while (!q.empty()) {
        auto v = q.front();
        q.pop();

        int vid = v.id();
        int dist = distance[vid];
        eval(v, dist);

        for (int i = 0; i < spellCount; ++i) {
            const auto& spell = spells[i];
            auto curSpell = spell;

            for (int j = 0; j < spell.maxTimes; ++j) {
                if (!(v < curSpell.delta)) {
                    Delta s = v + curSpell.delta;
                    int sid = s.id();

                    if (distance[sid] == -1) {
                        distance[sid] = dist + 1;
                        from[sid] = vid;
                        fromidx[sid] = i;
                        fromtimes[sid] = j + 1;
                        q.push(s);
                    }
                }
                curSpell.delta += spell.delta;
            }
        }
    }   

    assert(orderCost.size() == orders.size());
    int i = 0;
    std::vector<std::pair<float, int>> bestStates;
    for (const auto& [id, dist] : orderCost) {
        if (id != -1) {
            assert(dist > 0);
            debug(id, dist, distance[id], orders[i]);
            if (enemyOrdersDone < 0)
                bestStates.emplace_back(float(orders[i].price) / dist, id);
            else
                bestStates.emplace_back(100.f / dist + orders[i].price / 100.f, id);
        }
        ++i;
    }
    std::sort(bestStates.begin(), bestStates.end(), std::greater<std::pair<float, int>>());
    bestStates.resize(2);

    for (auto [value, id] : bestStates) {
        int dist = distance[id];
        debug(value, id, dist, Delta::decode(id));
        assert(dist >= 1);
        std::set<std::pair<int, int>> actionIdxs;

        while (dist > 0) {
            actionIdxs.insert({fromidx[id], fromtimes[id]});
            id = from[id];
            debug(Delta::decode(id));
            dist = distance[id];
        }

        assert(!actionIdxs.empty());
        for (const auto& [idx, times] : actionIdxs) {
            auto& spell = spells[idx];
            assert(times >= 1);
            if (spell.castable) {
                auto curSpell = spell;
                int targetTimes = -1;

                for (int i = 1; i <= times; ++i) {
                    if (!(player.inv < curSpell.delta))
                        targetTimes = i;
                    curSpell.delta += spell.delta;
                }

                if (targetTimes != -1) {
                    assert(targetTimes >= 1);
                    spell.times = targetTimes;
                    return &spell;
                }
            }
        }
    }

    return nullptr;
}

float Battle::eval(const Delta& v, int dist) {
    int maxPrice = 0;
    int vid = v.id();

    for (int i = 0; i < int(orders.size()); ++i) {
        const auto& order = orders[i];
        if (!(v < order.delta)) {
            if (dist < orderCost[i].dist)
                orderCost[i] = {vid, dist};
            if (maxPrice < order.price)
                maxPrice = order.price;
        }            
    }

    if (enemyOrdersDone < 4)
        return dist == 0 ? 0 : float(maxPrice);
    return dist == 0 ? 0 : 1.f / dist;
}
