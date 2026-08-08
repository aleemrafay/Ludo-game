#include "Dice.h"

Dice::Dice(int boxX_, int boxY_, int boxSize_)
    : boxX(boxX_), boxY(boxY_), boxSize(boxSize_), value(1), rolled(false) {
}

bool Dice::roll() {
    if (rolled) return false;
    value = SDL_rand(6) + 1; // SDL3: SDL_rand(n) -> [0, n)
    rolled = true;
    return true;
}

void Dice::resetForNewTurn() {
    rolled = false;
}

bool Dice::hitTest(float mx, float my) const {
    return mx >= boxX && mx <= boxX + boxSize &&
           my >= boxY && my <= boxY + boxSize;
}

void Dice::drawPip(SDL_Renderer* renderer, float nx, float ny) const {
    float cx = boxX + nx * boxSize;
    float cy = boxY + ny * boxSize;
    Color pipColor{ 20, 20, 20, 255 };
    GfxUtils::drawFilledCircle(renderer, cx, cy, 5.0f, pipColor);
}

void Dice::draw(SDL_Renderer* renderer) const {
    Color white{ 255, 255, 255, 255 };
    Color line{ 30, 30, 30, 255 };
    GfxUtils::drawCellRect(renderer, (float)boxX, (float)boxY, (float)boxSize, (float)boxSize, white, line);

    const float L = 0.22f, M = 0.5f, R = 0.78f;
    const float T = 0.22f, C = 0.5f, B = 0.78f;

    switch (value) {
    case 1: drawPip(renderer, M, C); break;
    case 2: drawPip(renderer, L, T); drawPip(renderer, R, B); break;
    case 3: drawPip(renderer, L, T); drawPip(renderer, M, C); drawPip(renderer, R, B); break;
    case 4: drawPip(renderer, L, T); drawPip(renderer, R, T); drawPip(renderer, L, B); drawPip(renderer, R, B); break;
    case 5: drawPip(renderer, L, T); drawPip(renderer, R, T); drawPip(renderer, M, C); drawPip(renderer, L, B); drawPip(renderer, R, B); break;
    case 6: drawPip(renderer, L, T); drawPip(renderer, R, T); drawPip(renderer, L, C); drawPip(renderer, R, C); drawPip(renderer, L, B); drawPip(renderer, R, B); break;
    }

    if (!rolled) {
        Color glow{ 80, 180, 80, 255 };
        SDL_SetRenderDrawColor(renderer, glow.r, glow.g, glow.b, glow.a);
        SDL_FRect ring = { (float)boxX - 3, (float)boxY - 3, (float)boxSize + 6, (float)boxSize + 6 };
        SDL_RenderRect(renderer, &ring);
    }
}
