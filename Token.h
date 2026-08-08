#pragma once
#include "Board.h"
#include <vector>

enum class TokenState { YARD, ON_PATH, HOME_STRETCH, FINISHED };

// Token: one playing piece. Encapsulates its own state (yard slot, path index,
// home-stretch index) and exposes behavior through methods rather than letting
// outside code poke at raw fields - Game/Player ask a Token "can you move N steps?"
// / "move N steps" and the Token figures out the rest using the shared Board layout.
class Token {
public:
    Token(int playerIndex, int tokenIndexInPlayer);

    // ---- Queries ----
    TokenState getState() const { return state; }
    int getPlayerIndex() const { return playerIndex; }
    int getTokenIndex() const { return tokenIndexInPlayer; }
    int getPathIndex() const { return pathIndex; }
    int getHomeStretchIndex() const { return homeStretchIndex; }
    int getStepsTaken() const { return stepsTaken; }

    // Can this token legally move `diceValue` steps right now, given board rules?
    bool canMove(const Board& board, int diceValue) const;

    // Current grid position (in cell units, e.g. col=3.5 means centered in column 3).
    // `yardSlotCenterProvider` positions are supplied by Player since yard-slot layout
    // is a per-player concern, not a Board concern.
    void getGridPos(const Board& board, float yardCenterCol, float yardCenterRow,
                     float& outCol, float& outRow) const;

    // ---- Mutating actions ----

    // Builds the sequence of grid cells this token will pass through if moved `steps`.
    // Does NOT mutate the token - used by Game to drive the slide animation first,
    // then applyMove() is called once the animation finishes.
    std::vector<GridPoint> buildMovePath(const Board& board, int steps) const;

    // Actually applies a move of `steps`, updating internal state. Call this once
    // the animation for that move has finished playing.
    void applyMove(const Board& board, int steps);

    // Sends this token back to its yard (used when captured/killed).
    void sendToYard();

private:
    int playerIndex;
    int tokenIndexInPlayer; // which of the player's 4 tokens this is (0-3), also its yard slot
    TokenState state;
    int pathIndex;        // 0-51 into Board's shared path, valid when ON_PATH
    int stepsTaken;        // steps taken since leaving the yard
    int homeStretchIndex;  // 0-4, valid when HOME_STRETCH
};
