#include "Action.hpp"
#include "Common.hpp"
#include "Options.hpp"

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

    auto repeatedDelta = delta;
    for (int i = 0; i < maxTimes; ++i) {
        repeatedDeltas[i] = repeatedDelta;
        repeatedDelta += delta;
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

std::istream& operator>>(std::istream& in, Witch& w) {
    return in >> w.inv >> w.score;
}

std::ostream& operator<<(std::ostream& out, const Witch& w) {
    return out << "delta=" << w.inv << ", " << "score=" << w.score;
}
