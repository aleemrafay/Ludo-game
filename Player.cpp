#include "Player.h"

std::string Player::nameForIndex(int index) {
    switch (index) {
    case 0: return "Red";
    case 1: return "Green";
    case 2: return "Yellow";
    case 3: return "Teal";
    default: return "Unknown";
    }
}

Player::Player(int playerIndex_)
    : playerIndex(playerIndex_), name(nameForIndex(playerIndex_)),
      tokens{ Token(playerIndex_, 0), Token(playerIndex_, 1),
              Token(playerIndex_, 2), Token(playerIndex_, 3) } {
}

void Player::getYardSlotCenter(const Board& board, int slot, float& outCol, float& outRow) const {
    const GridPoint& origin = board.yardOriginFor(playerIndex);
    int slotCol = slot % 2;
    int slotRow = slot / 2;

    float innerStartCol = origin.col + 1.5f;
    float innerStartRow = origin.row + 1.5f;
    float spacing = 2.0f;

    outCol = innerStartCol + slotCol * spacing + 1.0f;
    outRow = innerStartRow + slotRow * spacing + 1.0f;
}

bool Player::hasAnyLegalMove(const Board& board, int diceValue) const {
    for (const Token& t : tokens) {
        if (t.canMove(board, diceValue)) return true;
    }
    return false;
}

bool Player::hasWon() const {
    for (const Token& t : tokens) {
        if (t.getState() != TokenState::FINISHED) return false;
    }
    return true;
}
