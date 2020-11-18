

#include <iostream>

#if defined(LOCAL) && !defined(NDEBUG)
#define DEBUG
#endif

#ifdef DEBUG

constexpr int INF = 1e9;

inline void __debug(const char* s, const char*) {
    std::cerr << s << ": ";
}

template<typename T>
void __debug(const char* s, const T& x) {
    std::cerr << s << ": " << x << " ";
}

template<typename T, typename... Args>
void __debug(const char* s, const T& x, const Args&... rest)
{
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



#include <iostream>

using delta_t = int;

struct Delta {
    delta_t delta[4] = {0};

    static constexpr int LIMIT = 11;
    static constexpr int MAX_DELTA = LIMIT * LIMIT * LIMIT * LIMIT;

    inline int id() const {
        int id = 0;
        for (int i = 0; i < 4; ++i)
            id = id * LIMIT + delta[i];
        return id;
    }

    int& operator[](const int& idx);
    const int& operator[](const int& idx) const;
    Delta& operator+=(const Delta& o);
    bool operator<(const Delta& o) const;
    bool operator>(const Delta& o) const;

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

bool Delta::operator<(const Delta& o) const {
    int s = 0;
    for (int i = 0; i < 4; ++i) {
        int diff = delta[i] + o[i];
        if (diff < 0)
            return true;
        s += diff;
    }
    return s > LIMIT; 
}

bool Delta::operator>(const Delta& o) const {
    return o < *this;
}

Delta Delta::decode(int id) {
    Delta res;
    for (int i = 3; i >= 0; --i) {
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

    friend std::ostream& operator<<(std::ostream& out, const Order& order);
};

struct Spell : public Action {
    int maxTimes = 1;
    bool castable;
    bool repeatable;

    Spell(const int& id, const Delta& delta,
        const bool& castable, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Spell& spell);
};

struct Recipe : public Action {
    int tomeIndex;
    int taxCount;
    bool repeatable;

    Recipe(const int& id, const Delta& delta,
        const int &tomeIndex, const int& taxCount, const bool& repeatable);
    void print() const override;

    friend std::ostream& operator<<(std::ostream& out, const Recipe& recipe);
};

struct Rest : public Action {
	Rest();
    void print() const override;
};

struct Witch {
    Delta inv;
    int score;

    friend std::istream& operator>>(std::istream& in, Witch& witch);
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

std::ostream& operator<<(std::ostream& out, const Order& order) {
    return out << "ORDER: id=" << order.id 
        << ", delta=" << order.delta << ", "
        << "price=" << order.price;
}

Spell::Spell(const int& id, const Delta& delta,
    const bool& castable, const bool& repeatable) :
    Action(id, delta), castable(castable), repeatable(repeatable) {
    if (repeatable) {
        int times = 10;
        int s = 0;

        for (int i = 0; i < 4; ++i) {
            times = std::min(times, 10 / std::max(1, std::abs(delta[i])));
            s += delta[i];
        }
        times = std::min(times, 10 / std::max(1, std::abs(s)));

        debug(*this, times);
        maxTimes = times;        
    }
}

void Spell::print() const {
    std::cout << "CAST " << id << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Spell& spell) {
    return out << "SPELL: id=" << spell.id 
        << ", delta={" << spell.delta << ", "
        << "castable=" << spell.castable;
}

Recipe::Recipe(const int& id, const Delta& delta,
    const int& tomeIndex, const int& taxCount, const bool& repeatable) :
    Action(id, delta), tomeIndex(tomeIndex), taxCount(taxCount), repeatable(repeatable) {

}

void Recipe::print() const {
    std::cout << "LEARN " << id << std::endl;
}

std::ostream& operator<<(std::ostream& out, const Recipe& recipe) {
    return out << "RECIPE: id=" << recipe.id 
        << ", delta={" << recipe.delta << ", "
        << "tomeIndex=" << recipe.tomeIndex << ", "
        << "taxCount=" << recipe.taxCount << ", "
        << "repeatable=" << recipe.repeatable;
}

Rest::Rest() {

}

void Rest::print() const {
    std::cout << "REST" << std::endl;
}

std::istream& operator>>(std::istream& in, Witch& witch) {
    return in >> witch.inv >> witch.score;
}


#include <vector>

class Battle {
public:
    static void start();

private:
    static void readData();
    static const Action* pickAction();
    static const Action* getDoableOrder();
    static const Action* search();
    static float eval(const Delta& v, int d);

private:
    static Witch player;
    static Witch opponent;

    static std::vector<Spell> spells;
    static std::vector<Order> orders;
    static std::vector<Recipe> recipes;

    static Rest rest;

    static int dist[];
    static int from[];
    static int act[];

    static int roundNumber;
};


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

int Battle::dist[Delta::MAX_DELTA];
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
    std::fill(dist, dist + Delta::MAX_DELTA, -1);
    dist[player.inv.id()] = 0;
    std::queue<Delta> q;
    q.push(player.inv);

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

int main() {
	std::ios_base::sync_with_stdio(false);
	
    Battle::start();

    return 0;
}
