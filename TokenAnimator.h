#pragma once
#include <vector>
#include "Board.h"

// TokenAnimator: owns the state needed to smoothly slide one token, cell by
// cell, along a precomputed path. Game starts an animation by giving it the
// path; Game calls update() every frame; once finished(), Game applies the
// logical move (Token::applyMove) and asks for the interpolated position while
// it's running (for drawing).
class TokenAnimator {
public:
    TokenAnimator();

    // Begin animating `playerIndex`/`tokenIndex` along the given sequence of grid cells.
    // `startCol`/`startRow` is the token's current on-screen position (used as segment 0's start).
    void start(int playerIndex, int tokenIndex, const std::vector<GridPoint>& path,
               float startCol, float startRow, float speedCellsPerSecond);

    // Advances the animation by dt seconds. Returns true if the animation finished this frame.
    bool update(float dt);

    bool isRunning() const { return running; }
    int getAnimatingPlayer() const { return animPlayer; }
    int getAnimatingToken() const { return animToken; }
    int getStepCount() const { return (int)path.size(); }

    // Current interpolated grid position (only valid while running).
    void getInterpolatedPos(float& outCol, float& outRow) const;

private:
    bool running;
    int animPlayer, animToken;
    std::vector<GridPoint> path;
    int stepIndex;
    float progress; // 0..1 within the current segment
    float speed;    // segments per second
    float lastCol, lastRow; // position before the current step started (segment start)
};
