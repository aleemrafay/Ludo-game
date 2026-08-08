#include "Token.h"

Token::Token(int playerIndex_, int tokenIndexInPlayer_)
    : playerIndex(playerIndex_), tokenIndexInPlayer(tokenIndexInPlayer_),
      state(TokenState::YARD), pathIndex(-1), stepsTaken(-1), homeStretchIndex(-1) {
}

bool Token::canMove(const Board& board, int diceValue) const {
    if (state == TokenState::FINISHED) return false;
    if (state == TokenState::YARD) return diceValue == 6;

    int curSteps = stepsTaken;
    bool inHomeStretch = (state == TokenState::HOME_STRETCH);
    int curHomeIdx = homeStretchIndex;

    for (int s = 0; s < diceValue; s++) {
        if (!inHomeStretch) {
            curSteps++;
            if (curSteps > Board::STEPS_ON_SHARED_PATH) {
                inHomeStretch = true;
                curHomeIdx = curSteps - Board::STEPS_ON_SHARED_PATH - 1;
            }
        } else {
            curHomeIdx++;
            if (curHomeIdx > 4) {
                // Only a legal move if this is exactly the last step (must land exactly).
                return (s == diceValue - 1);
            }
        }
    }
    return true;
}

void Token::getGridPos(const Board& board, float yardCenterCol, float yardCenterRow,
                        float& outCol, float& outRow) const {
    switch (state) {
    case TokenState::YARD:
        outCol = yardCenterCol;
        outRow = yardCenterRow;
        break;
    case TokenState::ON_PATH: {
        const GridPoint& p = board.pathCellAt(pathIndex);
        outCol = p.col + 0.5f;
        outRow = p.row + 0.5f;
        break;
    }
    case TokenState::HOME_STRETCH: {
        const GridPoint& p = board.homeStretchCellAt(playerIndex, homeStretchIndex);
        outCol = p.col + 0.5f;
        outRow = p.row + 0.5f;
        break;
    }
    case TokenState::FINISHED:
        // Stack finished tokens near the center, slightly offset per player.
        outCol = 7.0f + (playerIndex % 2 == 0 ? -0.3f : 0.3f);
        outRow = 7.0f + (playerIndex < 2 ? -0.3f : 0.3f);
        break;
    }
}

std::vector<GridPoint> Token::buildMovePath(const Board& board, int steps) const {
    std::vector<GridPoint> outPath;

    if (state == TokenState::YARD) {
        // Leaving the yard always lands directly on the entry cell (steps must be 6).
        outPath.push_back(board.pathCellAt(board.entryIndexFor(playerIndex)));
        return outPath;
    }

    int stepsLeft = steps;
    int curSteps = stepsTaken;
    int curPathIdx = pathIndex;
    bool inHomeStretch = (state == TokenState::HOME_STRETCH);
    int curHomeIdx = homeStretchIndex;

    while (stepsLeft > 0) {
        if (!inHomeStretch) {
            curSteps++;
            if (curSteps > Board::STEPS_ON_SHARED_PATH) {
                inHomeStretch = true;
                curHomeIdx = curSteps - Board::STEPS_ON_SHARED_PATH - 1;
                outPath.push_back(board.homeStretchCellAt(playerIndex, curHomeIdx));
            } else {
                curPathIdx = (board.entryIndexFor(playerIndex) + curSteps - 1) % 52;
                outPath.push_back(board.pathCellAt(curPathIdx));
            }
        } else {
            curHomeIdx++;
            if (curHomeIdx > 4) {
                outPath.push_back(GridPoint{ 7, 7 }); // finish/center
                stepsLeft = 1; // force loop end after this push
            } else {
                outPath.push_back(board.homeStretchCellAt(playerIndex, curHomeIdx));
            }
        }
        stepsLeft--;
    }

    return outPath;
}

void Token::applyMove(const Board& board, int steps) {
    if (state == TokenState::YARD) {
        state = TokenState::ON_PATH;
        pathIndex = board.entryIndexFor(playerIndex);
        stepsTaken = 1;
        return;
    }

    int curSteps = stepsTaken;
    bool inHomeStretch = (state == TokenState::HOME_STRETCH);
    int curHomeIdx = homeStretchIndex;
    int curPathIdx = pathIndex;

    for (int s = 0; s < steps; s++) {
        if (!inHomeStretch) {
            curSteps++;
            if (curSteps > Board::STEPS_ON_SHARED_PATH) {
                inHomeStretch = true;
                curHomeIdx = curSteps - Board::STEPS_ON_SHARED_PATH - 1;
            } else {
                curPathIdx = (board.entryIndexFor(playerIndex) + curSteps - 1) % 52;
            }
        } else {
            curHomeIdx++;
        }
    }

    stepsTaken = curSteps;
    if (inHomeStretch) {
        if (curHomeIdx > 4) {
            state = TokenState::FINISHED;
            homeStretchIndex = -1;
        } else {
            state = TokenState::HOME_STRETCH;
            homeStretchIndex = curHomeIdx;
        }
    } else {
        state = TokenState::ON_PATH;
        pathIndex = curPathIdx;
    }
}

void Token::sendToYard() {
    state = TokenState::YARD;
    pathIndex = -1;
    stepsTaken = -1;
    homeStretchIndex = -1;
}
