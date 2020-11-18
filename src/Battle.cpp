#include "Battle.hpp"
#include "Common.hpp"

#include <queue>
#include <cassert>
#include <set>

Delta Battle::mInv, Battle::oInv;
std::vector<Spell> Battle::spells;
std::vector<Order> Battle::orders;
std::vector<Recipe> Battle::recipes;

int Battle::dist[Delta::MAX_DELTA];
int Battle::from[Delta::MAX_DELTA];
int Battle::act[Delta::MAX_DELTA];

float Battle::eval(const Delta& v, int d) {
    int maxPrice = 0;
    for (const auto& action : orders)
        if (!(v < action.delta) && maxPrice < action.price)
            maxPrice = action.price;
    return d == 0 ? 0 : float(maxPrice) / d;
}

int Battle::getOrder() {
    int id = -1, bestPrice = -1;
    for (const auto& action : orders)
        if (!(mInv < action.delta) && bestPrice < action.price) {
            bestPrice = action.price;
            id = action.id;
        }
    return id;
}

void Battle::pickAction() {
    int brewAction = getOrder();
    if (brewAction != -1) {
        std::cout << "BREW " << brewAction << std::endl;
        return;
    }

    std::fill(dist, dist + Delta::MAX_DELTA, -1);
    dist[mInv.id()] = 0;
    std::queue<Delta> q;
    q.push(mInv);

    float bestValue = -1.f;
    Delta bestState;

    while (!q.empty()) {
        auto v = q.front();
        q.pop();

        int vid = v.id();
        int d = dist[vid];

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
                if (dist[id] == -1) {
                    dist[id] = d + 1;
                    from[id] = vid;
                    act[id] = i;
                    q.push(s);
                }
            }
        }
    }

    debug(bestState, bestValue);

    int id = bestState.id();
    int d = dist[id];
    assert(d >= 1);
    std::set<int> actions;

    while (d > 0) {
        actions.insert(act[id]);
        id = from[id];
        d = dist[id];
    }

    for (int idx : actions)
        debug(spells[idx]);

    assert(!actions.empty());
    
    for (int idx : actions) {
        const auto& action = spells[idx];
        if (action.castable && !(mInv < action.delta)) {
            std::cout << "CAST " << action.id << std::endl;
            return;
        }
    }

    std::cout << "REST" << std::endl;
}

void Battle::start() {
    int roundIdx = 0;
    
    while (true) {
        int actionCount;
        std::cin >> actionCount;

        while (actionCount--) {
            int actionId;
            std::string actionStr;
            std::cin >> actionId >> actionStr;

            int delta[4];
            for (int i = 0; i < 4; ++i)
                std::cin >> delta[i];

            int price, tomeIndex, taxCount;
            bool castable, repeatable;
            std::cin >> price >> tomeIndex >> taxCount;
            std::cin >> castable >> repeatable;

            if (actionStr == "BREW")
                orders.push_back({
                    actionId,
                    {delta[0], delta[1], delta[2], delta[3]}, 
                    price});
            else if (actionStr == "CAST")
                spells.push_back({
                    actionId,
                    {delta[0], delta[1], delta[2], delta[3]},
                    castable
                });
            else if (actionStr == "LEARN")
                recipes.push_back({
                    actionId,
                    {delta[0], delta[1], delta[2], delta[3]},
                    tomeIndex,
                    taxCount,
                    repeatable
                });
        }     

        for (int i = 0; i < 4; ++i)
            std::cin >> mInv[i];
        int mScore; std::cin >> mScore;
        for (int i = 0; i < 4; ++i)
            std::cin >> oInv[i];
        int oScore; std::cin >> oScore;

        if (roundIdx < 5)
            std::cout << "LEARN " << recipes.front().id << std::endl;
        else
            pickAction();

        orders.clear();
        spells.clear();
        recipes.clear();
        ++roundIdx;
    }
}
