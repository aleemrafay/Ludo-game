#pragma once
#include <string>
#include <array>
#include "Token.h"
#include "Board.h"

// Player: owns 4 Tokens (composition) and knows its own index/name/color.
// Player index mapping: 0=Red, 1=Green, 2=Yellow, 3=Teal.
class Player {
public:
    static const int TOKENS_PER_PLAYER = 4;

    explicit Player(int playerIndex);

    int getIndex() const { return playerIndex; }
    const std::string& getName() const { return name; }

    Token& getToken(int tokenIndex) { return tokens[tokenIndex]; }
    const Token& getToken(int tokenIndex) const { return tokens[tokenIndex]; }

    // Yard slot center (in grid cell units) for one of this player's tokens,
    // arranged in a 2x2 pattern inside the player's 6x6 yard block.
    void getYardSlotCenter(const Board& board, int slot, float& outCol, float& outRow) const;

    // True if this player has at least one token that can legally move given diceValue.
    bool hasAnyLegalMove(const Board& board, int diceValue) const;

    // True if all 4 tokens have reached FINISHED (player has won).
    bool hasWon() const;

private:
    int playerIndex;
    std::string name;
    std::array<Token, TOKENS_PER_PLAYER> tokens;

    static std::string nameForIndex(int index);
};
