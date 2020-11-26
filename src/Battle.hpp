#ifndef BATTLE_HPP
#define BATTLE_HPP

#include "Delta.hpp"
#include "Action.hpp"
#include "Common.hpp"

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

#endif /* BATTLE_HPP */
