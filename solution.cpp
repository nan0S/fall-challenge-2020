#include <iostream>
#include <vector>
#include <numeric>
#include <queue>
#include <cassert>
#include <set>

#ifndef NDEBUG

void __debug(const char* s, const char* word) {
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

struct Delta {
    int delta[4];
    static constexpr int MAX_DELTA = 10 * 10 * 10 * 10;

    inline int id() const {
        int id = 0;
        for (int i = 0; i < 4; ++i)
            id = id * 10 + delta[i];
        return id;
    }

    int& operator[](const int& idx) {
        return delta[idx];
    }

    const int& operator[](const int& idx) const {
        return delta[idx];
    }

    Delta& operator+=(const Delta& o) {
        for (int i = 0; i < 4; ++i)
            delta[i] += o[i];
        return *this;
    }

    bool operator<(const Delta& o) const {
        int s = 0;
        for (int i = 0; i < 4; ++i) {
            int diff = delta[i] + o[i];
            if (diff < 0)
                return true;
            s += diff;
        }
        return s > 10; 
    }

    bool operator>(const Delta& o) const {
        return o < *this;
    }

    friend std::ostream& operator<<(std::ostream& out, const Delta& o) {
        return out << "{" << o.delta[0] << "," << o.delta[1] << ","
                   << o.delta[2] << "," << o.delta[3] << "}";
    }

    friend Delta operator+(const Delta& d1, const Delta& d2) {
        Delta res;
        for (int i = 0; i < 4; ++i)
            res[i] = d1[i] + d2[i];
        return res;
    }

    static Delta decode(int id) {
        Delta res;
        for (int i = 3; i >= 0; --i) {
            res[i] = id % 10;
            id /= 10;
        }
        assert(id == 0);
        return res;
    }
};

struct Order {
    int id;
    Delta delta;
    int price;

    friend std::ostream& operator<<(std::ostream& out, const Order& action) {
        return out << "BREW: id=" << action.id 
            << ", delta=" << action.delta << ", "
            << "price=" << action.price;
    }
};

struct Spell {
    int id;
    Delta delta;
    bool castable;

    friend std::ostream& operator<<(std::ostream& out, const Spell& action) {
        return out << "CAST: id=" << action.id 
            << ", delta={" << action.delta << ", "
            << "castable=" << action.castable;
    }
};

struct Recipe {
    int id;
    Delta delta;
    int tomeIndex;
    int taxCount;
    bool repeatable;
};

constexpr int INF = 1e9;

Delta mInv, oInv;
std::vector<Spell> spells;
std::vector<Order> orders;
std::vector<Recipe> recipes;

int dist[Delta::MAX_DELTA];
int from[Delta::MAX_DELTA];
int act[Delta::MAX_DELTA];

float eval(const Delta& v, int d) {
    int maxPrice = 0;
    for (const auto& action : orders)
        if (!(v < action.delta) && maxPrice < action.price)
            maxPrice = action.price;
    // return d == 0 ? 0 : 1.f / d;
    return d == 0 ? 0 : float(maxPrice) / d;
}

int getOrder() {
    int id = -1, bestPrice = -1;
    for (const auto& action : orders)
        if (!(mInv < action.delta) && bestPrice < action.price) {
            bestPrice = action.price;
            id = action.id;
        }
    return id;
}

void pickAction() {
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
        // if (value != 0.f)
            // debug(v, d, value);
        if (value > bestValue) {
            bestValue = value;
            bestState = v;
        }

        for (int i = 0; i < int(spells.size()); ++i) {
            const auto& action = spells[i];
            if (!(v < action.delta)) {
                Delta s = v + action.delta;
                // std::cerr << "\tBFS: " << s << std::endl;

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

int main() {
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

    return 0;
}
