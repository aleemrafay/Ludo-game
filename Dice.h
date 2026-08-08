#pragma once
#include <SDL3/SDL.h>
#include "GfxUtils.h"

// Dice: owns its current face value and whether it has been rolled this turn.
// Also knows how to draw itself (box + pips) and hit-test a click against its
// own screen position, keeping all dice-related concerns in one place.
class Dice {
public:
    Dice(int boxX, int boxY, int boxSize);

    int getValue() const { return value; }
    bool hasBeenRolled() const { return rolled; }

    // Rolls a new value (1-6) if not already rolled this turn. Returns true if a roll happened.
    bool roll();

    // Call at the start of a new turn so the dice can be rolled again.
    void resetForNewTurn();

    bool hitTest(float mx, float my) const;

    void draw(SDL_Renderer* renderer) const;

private:
    int boxX, boxY, boxSize;
    int value;
    bool rolled;

    void drawPip(SDL_Renderer* renderer, float nx, float ny) const;
};
