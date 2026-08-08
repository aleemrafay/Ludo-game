#include "GfxUtils.h"
#include <cmath>

void GfxUtils::drawFilledCircleWithRing(SDL_Renderer* renderer, float cx, float cy,
                                         float radius, const Color& fill) {
    // White ring behind the token (slightly larger radius) so it pops against
    // a same-colored background (e.g. a red token sitting on a red home cell).
    float ringRadius = radius + 3.0f;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (float dy = -ringRadius; dy <= ringRadius; dy++) {
        float dx = SDL_sqrtf(ringRadius * ringRadius - dy * dy);
        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }

    // Colored fill
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    for (float dy = -radius; dy <= radius; dy++) {
        float dx = SDL_sqrtf(radius * radius - dy * dy);
        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }

    // Dark outline ring
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    const int segments = 24;
    for (int i = 0; i < segments; i++) {
        float a1 = (float)i / segments * 2 * SDL_PI_F;
        float a2 = (float)(i + 1) / segments * 2 * SDL_PI_F;
        SDL_RenderLine(renderer,
            cx + radius * SDL_cosf(a1), cy + radius * SDL_sinf(a1),
            cx + radius * SDL_cosf(a2), cy + radius * SDL_sinf(a2));
    }
}

void GfxUtils::drawFilledCircle(SDL_Renderer* renderer, float cx, float cy,
                                 float radius, const Color& fill) {
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    for (float dy = -radius; dy <= radius; dy++) {
        float dx = SDL_sqrtf(radius * radius - dy * dy);
        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

void GfxUtils::drawCellRect(SDL_Renderer* renderer, float x, float y, float w, float h,
                             const Color& fill, const Color& outline) {
    SDL_FRect rect = { x, y, w, h };
    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, outline.r, outline.g, outline.b, outline.a);
    SDL_RenderRect(renderer, &rect);
}
