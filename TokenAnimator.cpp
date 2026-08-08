#include "TokenAnimator.h"

TokenAnimator::TokenAnimator()
    : running(false), animPlayer(-1), animToken(-1), stepIndex(0),
      progress(0.0f), speed(6.0f), lastCol(0), lastRow(0) {
}

void TokenAnimator::start(int playerIndex, int tokenIndex, const std::vector<GridPoint>& path_,
                           float startCol, float startRow, float speedCellsPerSecond) {
    if (path_.empty()) return;
    path = path_;
    animPlayer = playerIndex;
    animToken = tokenIndex;
    stepIndex = 0;
    progress = 0.0f;
    speed = speedCellsPerSecond;
    lastCol = startCol;
    lastRow = startRow;
    running = true;
}

bool TokenAnimator::update(float dt) {
    if (!running) return false;

    progress += speed * dt;
    while (progress >= 1.0f) {
        progress -= 1.0f;
        lastCol = (float)path[stepIndex].col + 0.5f;
        lastRow = (float)path[stepIndex].row + 0.5f;
        stepIndex++;
        if (stepIndex >= (int)path.size()) {
            running = false;
            return true; // finished this frame
        }
    }
    return false;
}

void TokenAnimator::getInterpolatedPos(float& outCol, float& outRow) const {
    if (!running || path.empty()) return;
    float toCol = (float)path[stepIndex].col + 0.5f;
    float toRow = (float)path[stepIndex].row + 0.5f;
    outCol = lastCol + (toCol - lastCol) * progress;
    outRow = lastRow + (toRow - lastRow) * progress;
}
