#include "Battle.hpp"

#include <cassert>
#include <algorithm>
#include <cstring>
#include <cmath>

bool State::operator<(const State& s) const {
    return evaluation < s.evaluation;
}

bool State::operator>(const State& s) const {
    return evaluation > s.evaluation;
}

std::ostream& operator<<(std::ostream& out, const State& s) {
    out << s.player << "\n";
    out << "gamma=" << s.gamma << "\n";
    out << "evaluation=" << s.evaluation << "\n";
    out << "ordersDone=" << s.ordersDone << "\n";
    out << "recipesLearnt=" << s.recipesLearnt << "\n";

    out << "SPELLS:\n";
    for (int i = 0; i < Battle::spellCount; ++i) {
        out << "\t" << (s.isCastable(i) ? "CASTABLE" : "NONCASTABLE") << " ";
        out << Battle::spells[i] << "\n";
    }

    out << "ORDERS:\n";
    for (int i = 0; i < Battle::orderCount; ++i) {
        out << "\t" << (s.isOrderDoable(i) ? "DOABLE" : "DONE") << " ";
        out << Battle::orders[i] << "\n";
    }

    out << "RECIPES:\n";
    for (int i = 0; i < Battle::recipeCount; ++i) {
        out << "\t" << (s.isRecipeDoable(i) ? "DOABLE" : "DONE") << " ";
        out << Battle::recipes[i] << "\n";
    }

    return out;
}

bool State::isCastable(const int& i) const {
    return castableSpellsMask & 1 << i;
}

bool State::isOrderDoable(const int& i) const {
    return ordersTodoMask & 1 << i;
}

bool State::isRecipeDoable(const int& i) const {
    return recipesTodoMask & 1 << i;
}

int State::getNeighbors(State* neighbors) const {
    int neighborCount = 0;

    if (ordersDone == 6) {
        getRestAction(neighbors, neighborCount);
        return neighborCount;
    }

    getSpellActions(neighbors, neighborCount);
    getOrderActions(neighbors, neighborCount);
    getRecipeActions(neighbors, neighborCount);
    getRestAction(neighbors, neighborCount);

    return neighborCount;
}

void State::getSpellActions(State* neighbors, int& neighborCount) const {
    int castableSpellsMask = this->castableSpellsMask;
    while (castableSpellsMask) {
        int nextSpellBit = low(castableSpellsMask);
        assert(__builtin_popcount(nextSpellBit) == 1);

        int i = bits(nextSpellBit);
        assert(nextSpellBit == (1 << i));
        assert(0 <= i && i < Battle::spellCount);

        const auto& s = Battle::spells[i];
        for (int j = 0; j < s.maxTimes; ++j) {
            const auto& delta = s.repeatedDeltas[j];
            if (!player.inv.canApply(delta))
                break;

            auto& neighbor = neighbors[neighborCount++];
            std::memcpy(&neighbor, this, sizeof(State));
            neighbor.player.inv += delta;
            neighbor.castableSpellsMask ^= nextSpellBit;
            neighbor.gamma *= DECAY;
            neighbor.evaluation += delta.eval() - 0.01f;

            if (firstAction == nullptr) {
                auto& customSpell = Battle::customSpells[Battle::customSpellCount++];
                customSpell = s;
                customSpell.curTimes = j + 1;
                neighbor.firstAction = &customSpell;
            }
        }

        assert((castableSpellsMask & nextSpellBit) == nextSpellBit);
        castableSpellsMask ^= nextSpellBit;
    }
}

void State::getOrderActions(State* neighbors, int& neighborCount) const {
    int ordersTodoMask = this->ordersTodoMask;
    while (ordersTodoMask) {
        int nextOrderBit = low(ordersTodoMask);
        assert(__builtin_popcount(nextOrderBit) == 1);

        int i = bits(nextOrderBit);
        assert(nextOrderBit == (1 << i));
        assert(0 <= i && i < Battle::orderCount);

        const auto& order = Battle::orders[i];
        if (player.inv.canApply(order.delta)) {
            auto& neighbor = neighbors[neighborCount++];
            std::memcpy(&neighbor, this, sizeof(State));
            neighbor.player.inv += order.delta;
            neighbor.player.score += order.price;
            neighbor.ordersTodoMask ^= nextOrderBit;
            neighbor.gamma *= DECAY;
            neighbor.evaluation += 100 * gamma * order.price;
            if (++neighbor.ordersDone == 6)
                neighbor.evaluation += 1e4;

            if (firstAction == nullptr)
                neighbor.firstAction = &order;
        }

        assert((ordersTodoMask & nextOrderBit) == nextOrderBit);
        ordersTodoMask ^= nextOrderBit;
    }
}

void State::getRecipeActions(State* neighbors, int& neighborCount) const {
    for (int i = 0; i < Battle::recipeCount; ++i)
        if (recipesTodoMask & 1 << i) {
            const auto& recipe = Battle::recipes[i];

            if (player.inv[0] >= recipe.tomeIndex) {
                auto& neighbor = neighbors[neighborCount++];
                std::memcpy(&neighbor, this, sizeof(State));
                neighbor.recipesTodoMask ^= 1 << i;
                neighbor.gamma *= DECAY;
                neighbor.evaluation += gamma * std::pow(LEARN_DECAY, recipesLearnt) *
                    (1 - recipe.tomeIndex / 3.f + recipe.taxCount / 6.f);
                neighbor.recipesLearnt++;

                if (firstAction == 0)
                    neighbor.firstAction = &recipe;
            }
        }
        else if (castableSpellsFromRecipesMask & 1 << i) {
            const auto& s = Battle::spellsFromRecipes[i];
            for (int j = 0; j < s.maxTimes; ++j) {
                const auto& delta = s.repeatedDeltas[j];
                if (!player.inv.canApply(delta))
                    break;

                auto& neighbor = neighbors[neighborCount++];
                std::memcpy(&neighbor, this, sizeof(State));
                neighbor.player.inv += delta;
                neighbor.castableSpellsFromRecipesMask ^= 1 << i;
                neighbor.gamma *= DECAY;
                neighbor.evaluation += delta.eval() - 0.01f;

                assert(firstAction != nullptr);
            }
        }
}

void State::getRestAction(State* neighbors, int& neighborCount) const {
    auto& neighbor = neighbors[neighborCount++];
    std::memcpy(&neighbor, this, sizeof(State));
    int turnOnCount = Battle::spellCount - __builtin_popcount(neighbor.castableSpellsMask) +
        Battle::recipeCount - __builtin_popcount(neighbor.castableSpellsFromRecipesMask);
    neighbor.castableSpellsMask = (1 << Battle::spellCount) - 1;
    neighbor.castableSpellsFromRecipesMask = (1 << Battle::recipeCount) - 1;
    neighbor.gamma *= DECAY;
    neighbor.evaluation += turnOnCount * 0.01f;

    if (firstAction == nullptr)
        neighbor.firstAction = &Battle::rest;
}

int Battle::spellCount;
int Battle::orderCount;
int Battle::recipeCount;
int Battle::customSpellCount = 0;

std::array<Spell, Battle::MAX_SPELL_COUNT> Battle::spells;
std::array<Order, Battle::MAX_ORDER_COUNT> Battle::orders;
std::array<Recipe, Battle::MAX_RECIPE_COUNT> Battle::recipes;
std::array<Spell, Battle::MAX_SPELL_COUNT> Battle::customSpells;
std::array<Spell, Battle::MAX_RECIPE_COUNT> Battle::spellsFromRecipes;
Rest Battle::rest;

int Battle::playerOrdersDone = 0;
int Battle::enemyOrdersDone = 0;

Witch Battle::player;
Witch Battle::opponent;

int Battle::roundNumber = 0;
int Battle::recipeDoneCount = 0;

void Battle::start() {
    while (true) {
        resetData();
        readData();
        // #ifdef DEBUG
        // writeData();
        // #endif

        const Action* action = pickAction();
        if (dynamic_cast<const Recipe*>(action)) {
            debug("MAKING RECIPE");
            ++recipeDoneCount;
            debug(recipeDoneCount);
        }
        action->print();

        ++roundNumber;
    }
}

void Battle::resetData() {
    spellCount = orderCount = recipeCount = customSpellCount = 0;
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
            orders[orderCount++] = Order(actionId, delta, price);
        else if (actionStr == "CAST")
            spells[spellCount++] = Spell(actionId, delta, castable, repeatable);
        else if (actionStr == "LEARN") {
            recipes[recipeCount] = Recipe(actionId, delta, tomeIndex, taxCount, repeatable);
            spellsFromRecipes[recipeCount] = recipes[recipeCount];
            ++recipeCount;
        }
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
        ++enemyOrdersDone;
    }

    assert(spellCount <= MAX_SPELL_COUNT);
    assert(orderCount <= MAX_ORDER_COUNT);
    assert(recipeCount <= MAX_RECIPE_COUNT);
}

#ifdef DEBUG
void Battle::writeData() {
    for (const auto& order : orders)
        debug(order);
    for (const auto& spell : spells)
        debug(spell);
    for (const auto& recipe : recipes)
        debug(recipe);
    for (const auto& spellFromRecipe : spellsFromRecipes)
        debug(spellFromRecipe);
}
#endif

const Action* Battle::pickAction() {
    // if (roundNumber < 6)
        // return chooseRecipe();
    return search();
}

const Action* Battle::chooseRecipe() {
    return &recipes.front();
}

const Action* Battle::search() {
    static constexpr int MAX_STATES = BEAM_WIDTH * State::MAX_NEIGHBORS;
    static std::array<State, MAX_STATES> current, next;
    int currentCount = 1, nextCount = 0;
    current[0] = getInitialState();

    std::array<State, State::MAX_NEIGHBORS> neighbors;

    float timeLimit = roundNumber == 0 ? 1000 : 50;
    int depth = 0;

    for (Timer timer(timeLimit); timer.isTimeLeft(); ++depth) {       
        assert(currentCount > 0);

        int considerCount = std::min(BEAM_WIDTH, currentCount);
        for (int i = 0; i < considerCount; ++i) {
            const auto& state = current[i];
            nextCount += state.getNeighbors(next.data() + nextCount);
        }

        assert(nextCount > 0);
        considerCount = std::min(BEAM_WIDTH, nextCount);
        std::partial_sort(next.data(),
            next.data() + considerCount,
            next.data() + nextCount,
            std::greater<State>());

        std::swap(current, next);
        currentCount = nextCount;
        nextCount = 0;
    }

    assert(currentCount > 0);
    const auto& finalState = current[0];
    debug(finalState);
    assert(finalState.firstAction != nullptr);
    debug("Beam search depth:", depth);

    assert(customSpellCount <= MAX_SPELL_COUNT);

    return finalState.firstAction;
}

State Battle::getInitialState() {
    State initialState;
    initialState.player.inv = player.inv;
    initialState.player.score = 0;

    initialState.castableSpellsMask = 0;
    for (int i = 0; i < spellCount; ++i)
        if (spells[i].castable)
            initialState.castableSpellsMask |= 1 << i;

    initialState.ordersTodoMask = (1 << orderCount) - 1;
    initialState.recipesTodoMask = (1 << recipeCount) - 1;
    initialState.castableSpellsFromRecipesMask = (1 << recipeCount) - 1;
    initialState.gamma = 1.f;

    initialState.evaluation = initialState.player.inv.eval() +
        __builtin_popcount(initialState.castableSpellsMask) * 0.01f;

    initialState.ordersDone = playerOrdersDone;
    initialState.recipesLearnt = recipeDoneCount;

    return initialState;
}
