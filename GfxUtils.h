#pragma once
#include <SDL3/SDL.h>

// Simple RGBA color struct shared across the project.
struct Color {
    Uint8 r, g, b, a;
};

// GfxUtils: a small collection of static drawing helpers built on top of raw
// SDL_Renderer calls. SDL3 has no built-in filled-circle primitive, so these
// wrap the "scanline" approach used throughout the project. Kept as a static
// utility class (no state) so every other class can reuse the same drawing
// code without duplicating it or relying on free functions.
class GfxUtils {
public:
    // Filled circle with a white "halo" ring behind it and a dark outline in front,
    // used for drawing tokens so they stay visible against same-colored backgrounds.
    static void drawFilledCircleWithRing(SDL_Renderer* renderer, float cx, float cy,
                                          float radius, const Color& fill);

    // Plain filled circle, no ring/outline (used for small pips/badges).
    static void drawFilledCircle(SDL_Renderer* renderer, float cx, float cy,
                                  float radius, const Color& fill);

    // Axis-aligned filled rectangle + outline, used for board cells.
    static void drawCellRect(SDL_Renderer* renderer, float x, float y, float w, float h,
                              const Color& fill, const Color& outline);
};
