#include "Battle.hpp"
#include "Options.hpp"
#include "Common.hpp"

#include <queue>
#include <cassert>
#include <set>
#include <vector>
#include <algorithm>
#include <math.h>
#include <array>
#include <cstring>

bool State::operator<(const State& s) const {
    // return eval() < s.eval();
    return player.score < s.player.score;
}

std::ostream& operator<<(std::ostream& out, const State& s) {
    out << s.player << "\n";
    out << "ordersDone: " << s.getOrdersDone() << "\n";

    for (const auto& hist : s.history)
        out << hist << " ";

    // int spellCount = Battle::spells.size();
    // out << "Spells:\n";
    // for (int i = 0; i < spellCount; ++i) {
    //     auto& spell = Battle::spells[i];
    //     auto tmp = spell.castable;
    //     spell.castable = s.isCastable(i);
    //     out << "\t" << spell << "\n";
    //     spell.castable = tmp;
    // }

    // int orderCount = Battle::orders.size();
    // out << "Orders:\n";
    // for (int i = 0; i < orderCount; ++i) {
    //     const auto& order = Battle::orders[i];
    //     out << "\t" << (s.isOrderDone(i) ? "DONE " : "UNDONE ") << order << "\n";
    // }
    return out;
}

int State::getNeighbors(std::array<State, MAX_NEIGHBORS>& neighbours) const {

    int neighborCount = 0;

    int ordersUndoneMask = this->ordersUndoneMask;
    while (ordersUndoneMask) {
        int nextOrderBit = low(ordersUndoneMask);
        int i = bits(nextOrderBit);
        const auto& order = Battle::orders[i];

        if (player.inv.canApply(order.delta)) {
            auto& neighbor = neighbours[neighborCount++];
            neighbor = *this;
            neighbor.player.inv = player.inv + order.delta;
            neighbor.player.score =  player.score + 100 * gamma * order.price + gamma * order.delta.eval();
            neighbor.castableSpellsMask = castableSpellsMask;
            neighbor.ordersUndoneMask = this->ordersUndoneMask ^ nextOrderBit;
            neighbor.gamma = gamma * DECAY;

            neighbor.history.push_back(player.inv);

            if (firstAction == nullptr)
                neighbor.firstAction = &Battle::orders[i];
        }

        ordersUndoneMask ^= nextOrderBit;
    }

    int castableSpellsMask = this->castableSpellsMask;
    while (castableSpellsMask) {
        int nextSpellBit = low(castableSpellsMask);
        int i = bits(nextSpellBit);
        const auto& s = Battle::spells[i];

        for (int times = 0; times < s.maxTimes; ++times) {
            const auto& repeatedDelta = s.repeatedDeltas[times];

            if (player.inv.canApply(repeatedDelta)) {
                auto& neighbor = neighbours[neighborCount++];
                // std::memcpy(&neighbor, this, sizeof(State));
                neighbor = *this;
                neighbor.player.inv += repeatedDelta;
                neighbor.player.score += repeatedDelta.eval() * gamma;
                neighbor.castableSpellsMask ^= nextSpellBit;
                neighbor.gamma *= DECAY;

                neighbor.history = history;
                neighbor.history.push_back(player.inv);

                if (firstAction == nullptr) {
                    Battle::tSpells.push_back(s);
                    Battle::tSpells.back().curTimes = times + 1;
                    neighbor.firstAction = &Battle::tSpells.back();
                }
            }
        }

        castableSpellsMask ^= nextSpellBit;
    }

    auto& neighbor = neighbours[neighborCount++];
    // std::memcpy(&neighbor, this, sizeof(State)); 
    neighbor = *this;
    neighbor.castableSpellsMask = (1 << int(Battle::spells.size())) - 1;
    neighbor.gamma *= DECAY;

    neighbor.history = history;
    neighbor.history.push_back(player.inv);
    if (firstAction == nullptr)
        neighbor.firstAction = &Battle::rest;

    assert(neighborCount <= MAX_NEIGHBORS);

    return neighborCount;
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
        // #ifdef DEBUG
        // writeData();
        // #endif
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

    static float lastPlayerScore = 0;
    if (player.score != lastPlayerScore) {
        lastPlayerScore = player.score;
        ++playerOrdersDone;
    }

    static float lastEnemyScore = 0;
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

    float timeLimit = roundNumber == 0 ? 1000 : 50;
    std::array<State, State::MAX_NEIGHBORS> neighbors;
    int depth = 0;

    for (Timer timer(timeLimit); timer.isTimeLeft(); ++depth) {       
        assert(!q.empty());
        // debug("DEPTH:", depth, q.top());

        for (int i = 0; i < beamWidth; ++i) {
            const auto state = q.top();
            q.pop();

            int neighborCount = state.getNeighbors(neighbors);
            for (int i = 0; i < neighborCount; ++i)
                layer.push(neighbors[i]);

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
    debug("Beam search depth:", depth);

    return finalState.firstAction;
}

State Battle::getInitialState() {
    State initialState;
    initialState.player = player;
    initialState.player.score = 0.f;
    initialState.firstAction = nullptr;
    initialState.castableSpellsMask = 0;
    for (int i = 0; i < int(spells.size()); ++i)
        if (spells[i].castable)
            initialState.castableSpellsMask |= 1 << i;
    initialState.ordersUndoneMask = (1 << int(orders.size())) - 1;
    initialState.gamma = 1.f;
    return initialState;
}
