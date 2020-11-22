#include "Action.hpp"
#include "Common.hpp"

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

        maxTimes = times;        
    }
}

void Spell::print() const {
    assert(times >= 1);
    std::cout << "CAST " << id << " " << times << std::endl;
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
