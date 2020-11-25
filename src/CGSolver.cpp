#define DEBUG


#include <iostream>
#include <chrono>

using eval_t = float;

constexpr int INF = 1e9;

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

namespace Options {
	extern int enemyOrdersDone;
}


int Options::enemyOrdersDone = 0;


#include <iostream>

using delta_t = int;

struct Delta {
    delta_t delta[4] = {0};

    static constexpr int LIMIT = 11;

    bool canApply(const Delta& d) const;
    eval_t eval() const;

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);

    friend std::istream& operator>>(std::istream& in, Delta& d);
    friend std::ostream& operator<<(std::ostream& out, const Delta& o);
    friend Delta operator+(const Delta& d1, const Delta& d2);

    static Delta decode(int id);
};


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

eval_t Delta::eval() const {
    eval_t value = 0;
    for (int i = 0; i < 4; ++i)
        value += delta[i] * (i + 2);
    return value;
}

Delta Delta::decode(int id) {
    Delta res;
    for (int i = 0; i < 4; ++i) {
        res[i] = id % LIMIT;
        id /= LIMIT;
    }
    assert(id == 0);
    return res;
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

    Order(const int& id, const Delta& delta, const int& price);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Order& o);
};

struct Spell : public Action {
    int maxTimes = 1;
    int curTimes = 1;
    bool castable;
    bool repeatable;

    Spell(const int& id, const Delta& delta,
        const bool& castable, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Spell& s);
};

struct Recipe : public Action {
    int tomeIndex;
    int taxCount;
    bool repeatable;

    Recipe(const int& id, const Delta& delta,
        const int &tomeIndex, const int& taxCount, const bool& repeatable);
    void print() const override;

    eval_t eval() const;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& r);
};

struct Rest : public Action {
	Rest();
    void print() const override;
};

struct Witch {
    Delta inv;
    int score;

    eval_t eval() const;

    friend std::istream& operator>>(std::istream& in, Witch& w);
    friend std::ostream& operator<<(std::ostream& out, const Witch& w);
};


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

Recipe::Recipe(const int& id, const Delta& delta,
    const int& tomeIndex, const int& taxCount, const bool& repeatable) :
    Action(id, delta), tomeIndex(tomeIndex), taxCount(taxCount), repeatable(repeatable) {

}

void Recipe::print() const {
    std::cout << "LEARN " << id << std::endl;
}

eval_t Recipe::eval() const {
    return delta.eval() + repeatable * 5;
}

std::ostream& operator<<(std::ostream& out, const Recipe& r) {
    return out << "RECIPE: id=" << r.id 
        << ", delta={" << r.delta << ", "
        << "tomeIndex=" << r.tomeIndex << ", "
        << "taxCount=" << r.taxCount << ", "
        << "repeatable=" << r.repeatable;
}

Rest::Rest() {

}

void Rest::print() const {
    std::cout << "REST" << std::endl;
}

eval_t Witch::eval() const {
    eval_t value = score * 3;
    value += inv.eval();
    return value;
}

std::istream& operator>>(std::istream& in, Witch& w) {
    return in >> w.inv >> w.score;
}

std::ostream& operator<<(std::ostream& out, const Witch& w) {
    return out << "delta=" << w.inv << ", " << "score=" << w.score;
}


#include <vector>

struct State {
    Witch player;

    std::vector<Spell> spells;
    std::vector<Order> orders;
    std::vector<Recipe> recipes;

    const Action* firstAction = nullptr;
    int ordersDone = 0;

    std::vector<State> getNeighbors() const;
    eval_t eval() const;

    bool operator<(const State& s) const;
    friend std::ostream& operator<<(std::ostream& out, const State& s);
};

class Battle {
public:
    static void start();

public:
    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;
    static Rest rest;

    static std::vector<Spell> tSpells;

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

    static constexpr int beamDepth = 5;
    static constexpr int beamWidth = 5;
    static constexpr int recipeConsider = 2;
};


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

    float timeLimit = roundNumber == 0 ? 1000 : 50;
    int depth = 0;

    for (Timer timer(timeLimit); timer.isTimeLeft(); ++depth) {       
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
    debug("Beam search depth:", depth);

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

int main() {
	std::ios_base::sync_with_stdio(false);
	
    Battle::start();

    return 0;
}
