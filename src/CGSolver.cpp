#define DEBUG


#include <iostream>
#include <chrono>

using eval_t = float;

constexpr int INF = 1e9;

#define low(x) ((x) & (-(x))) // lowest bit
#define bits(x) (31 - __builtin_clz(x)) // floor(log2(x))

#if defined(LOCAL) && !defined(NDEBUG)
#define DEBUG
#endif

#ifdef DEBUG

inline void __debug(const char* s, const char*) {
    std::cerr << s << ": ";
}

template<typename T>
void __debug(const char* s, const T& x) {
    std::cerr << s << ": " << x << " ";
}

template<typename T, typename... Args>
void __debug(const char* s, const T& x, const Args&... rest) {
    int bracket = 0;
    char c;
    while ((c = *s) != ',' || bracket)
    {
        std::cerr << *s++;
        switch (c)
        {
            case '(':
            case '{':
            case '[':
                ++bracket;
                break;
            case ')':
            case '}':
            case ']':
                --bracket;
        }
    }
    std::cerr << ": ";
    std::cerr << x << ",";
    __debug(s + 1, rest...);

}

template<typename... Args>
void _debug(const char* s, const Args&... rest) {
    __debug(s, rest...);
    std::cerr << std::endl;
}

#define debug(...) _debug(#__VA_ARGS__, __VA_ARGS__)
#else
#define debug(...) 13
#endif

class Timer {
public:
    Timer(float timeLimit);
    bool isTimeLeft() const;

private:
    float timeLimit;
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
};


Timer::Timer(float timeLimit) :
	timeLimit(timeLimit), startTime(std::chrono::high_resolution_clock::now()) {

}

bool Timer::isTimeLeft() const {
	auto now = std::chrono::high_resolution_clock::now();
	return std::chrono::duration<float>(now - startTime).count() * 1000 < timeLimit;
}


#include <iostream>

using delta_t = int;

struct Delta {
    delta_t delta[4] = {0};

    bool canApply(const Delta& d) const;
    inline eval_t eval() const;

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);

    friend std::istream& operator>>(std::istream& in, Delta& d);
    friend std::ostream& operator<<(std::ostream& out, const Delta& o);
    friend Delta operator+(const Delta& d1, const Delta& d2);
};

eval_t Delta::eval() const {
    eval_t value = 0;
    for (int i = 0; i < 4; ++i)
        value += delta[i] * (i + 1);
    return value;
}


#include <cassert>

int& Delta::operator[](const int& idx) {
    return delta[idx];
}

const int& Delta::operator[](const int& idx) const {
    return delta[idx];
}

Delta& Delta::operator+=(const Delta& o) {
    for (int i = 0; i < 4; ++i)
        delta[i] += o[i];
    return *this;
}

bool Delta::canApply(const Delta& d) const {
    int s = 0;
    for (int i = 0; i < 4; ++i) {
        int diff = delta[i] + d[i];
        if (diff < 0)
            return false;
        s += diff;
    }
    assert(s >= 0);
    return s <= 10; 
}

std::istream& operator>>(std::istream& in, Delta& d) {
    return in >> d.delta[0] >> d.delta[1]
              >> d.delta[2] >> d.delta[3];
}

std::ostream& operator<<(std::ostream& out, const Delta& o) {
    return out << "{" << o.delta[0] << "," << o.delta[1] << ","
               << o.delta[2] << "," << o.delta[3] << "}";
}

Delta operator+(const Delta& d1, const Delta& d2) {
    Delta res;
    for (int i = 0; i < 4; ++i)
        res[i] = d1[i] + d2[i];
    return res;
}


#include <array>

struct Action {
    Delta delta;
    int id;

    Action() = default;
    Action(const int& id, const Delta& delta);

    virtual void print() const = 0;
    virtual ~Action() = default;
};

struct Order : public Action {
    int price;

    Order() = default;
    Order(const int& id, const Delta& delta, const int& price);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Order& o);
};

struct Recipe : public Action {
    int tomeIndex;
    int taxCount;
    bool repeatable;

    Recipe() = default;
    Recipe(const int& id, const Delta& delta,
        const int &tomeIndex, const int& taxCount, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& r);
};

struct Spell : public Action {
    bool castable;
    bool repeatable;
    int maxTimes = 1;
    int curTimes = 1;

    static constexpr int MAX_REPEATED_DELTA = 10;
    std::array<Delta, MAX_REPEATED_DELTA> repeatedDeltas;

    Spell() = default;
    Spell(const int& id, const Delta& delta,
        const bool& castable, const bool& repeatable);
    Spell(const Recipe& recipe);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Spell& s);
};

struct Rest : public Action {
	Rest() = default;
    void print() const override;
};

struct Witch {
    Delta inv;
    int score;

    inline eval_t eval() const;

    friend std::istream& operator>>(std::istream& in, Witch& w);
    friend std::ostream& operator<<(std::ostream& out, const Witch& w);
};

eval_t Witch::eval() const {
    eval_t value = score;
    value += inv.eval();
    return value;
}


#include <cassert>

Action::Action(const int& id, const Delta& delta) :
    delta(delta), id(id) {

}

Order::Order(const int& id, const Delta& delta, const int& price) :
    Action(id, delta), price(price) {

}

void Order::print() const {
    std::cout << "BREW " << id << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Order& o) {
    return out << "ORDER: id=" << o.id 
        << ", delta=" << o.delta << ", "
        << "price=" << o.price;
}

Recipe::Recipe(const int& id, const Delta& delta,
    const int& tomeIndex, const int& taxCount, const bool& repeatable) :
    Action(id, delta), tomeIndex(tomeIndex), taxCount(taxCount), repeatable(repeatable) {

}

void Recipe::print() const {
    std::cout << "LEARN " << id << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Recipe& r) {
    return out << "RECIPE: id=" << r.id 
        << ", delta={" << r.delta << ", "
        << "tomeIndex=" << r.tomeIndex << ", "
        << "taxCount=" << r.taxCount << ", "
        << "repeatable=" << r.repeatable;
}

Spell::Spell(const int& id, const Delta& delta,
    const bool& castable, const bool& repeatable) :
    Action(id, delta), castable(castable), repeatable(repeatable) {

    if (repeatable) {
        int provide = 0, supply = 0;
        for (int i = 0; i < 4; ++i)
            if (delta[i] < 0)
                provide -= delta[i];
            else
                supply += delta[i];

        assert(provide >= 0 && supply >= 0);

        maxTimes = 10;
        if (provide > 0)
            maxTimes = std::min(maxTimes, 10 / provide);
        if (supply > 0)
            maxTimes = std::min(maxTimes, 10 / supply);
    }

    assert(maxTimes <= MAX_REPEATED_DELTA);

    repeatedDeltas[0] = delta;
    for (int i = 1; i < maxTimes; ++i)
        repeatedDeltas[i] = repeatedDeltas[i - 1] + delta;
}

Spell::Spell(const Recipe& recipe) :
    Spell(recipe.id, recipe.delta, true, recipe.repeatable) {

}

void Spell::print() const {
    assert(curTimes >= 1);
    std::cout << "CAST " << id << " " << curTimes << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Spell& s) {
    return out << "SPELL: id=" << s.id 
        << ", delta=" << s.delta << ", "
        << "castable=" << s.castable << ", "
        << "maxTimes=" << s.maxTimes;
}

void Rest::print() const {
    std::cout << "REST" << std::endl;
}

std::istream& operator>>(std::istream& in, Witch& w) {
    return in >> w.inv >> w.score;
}

std::ostream& operator<<(std::ostream& out, const Witch& w) {
    return out << "delta=" << w.inv << ", " << "score=" << w.score;
}


#include <array>

struct State;

class Battle {
public:
    static void start();

private:
    static void resetData();
    static void readData();
    #ifdef DEBUG
    static void writeData();
    #endif
    static const Action* pickAction();
    static const Action* chooseRecipe();
    static const Action* search();
    static State getInitialState();

public:
    static int spellCount;
    static int orderCount;
    static int recipeCount;
    static int customSpellCount;

    static constexpr int MAX_SPELL_COUNT = 20;
    static constexpr int MAX_ORDER_COUNT = 5;
    static constexpr int MAX_RECIPE_COUNT = 6;

    static std::array<Spell, MAX_SPELL_COUNT> spells;
    static std::array<Order, MAX_ORDER_COUNT> orders;
    static std::array<Recipe, MAX_RECIPE_COUNT> recipes;
    static std::array<Spell, MAX_SPELL_COUNT> customSpells;
    static std::array<Spell, MAX_RECIPE_COUNT> spellsFromRecipes;
    static Rest rest;

    static int playerOrdersDone;
    static int enemyOrdersDone;

    static Witch player;
    static Witch opponent;

    static int roundNumber;
    static int recipeDoneCount;
    static constexpr int BEAM_WIDTH = 2000;
};

struct State {
    Witch player;
    int castableSpellsMask;
    int ordersTodoMask;
    int recipesTodoMask;
    int castableSpellsFromRecipesMask;
    float gamma;
    eval_t evaluation;
    int ordersDone;
    int recipesLearnt;

    const Action* firstAction;

    static constexpr int MAX_NEIGHBORS = 30;
    static constexpr float DECAY = 0.97f;
    static constexpr float LEARN_DECAY = 0.6f;

    int getNeighbors(State* neighbors) const;
    void getSpellActions(State* neighbors, int& neighborCount) const;
    void getOrderActions(State* neighbors, int& neighborCount) const;
    void getRecipeActions(State* neighbors, int& neighborCount) const;
    void getRestAction(State* neighbors, int& neighborCount) const;

    friend std::ostream& operator<<(std::ostream& out, const State& s);
    bool isCastable(const int& i) const;
    bool isOrderDoable(const int& i) const;
    bool isRecipeDoable(const int& i) const;

    bool operator<(const State& s) const;
    bool operator>(const State& s) const;
};


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

int main() {
	std::ios_base::sync_with_stdio(false);
	
    Battle::start();

    return 0;
}
