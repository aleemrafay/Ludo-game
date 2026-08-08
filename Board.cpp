#include "Board.h"
#include <cmath>

Board::Board() {
    buildPath();
}

void Board::buildPath() {
    int i = 0;
    // Segment 1: Red's row (row 6), cols 1-5
    for (int c = 1; c <= 5; c++) pathCells[i++] = { c, 6 };
    // Segment 2: column 6, rows 5 down to 0
    for (int r = 5; r >= 0; r--) pathCells[i++] = { 6, r };
    // Segment 3: top corner
    pathCells[i++] = { 7, 0 };
    // Segment 4: column 8, rows 0 to 5
    for (int r = 0; r <= 5; r++) pathCells[i++] = { 8, r };
    // Segment 5: Green's row (row 6), cols 9-14
    for (int c = 9; c <= 14; c++) pathCells[i++] = { c, 6 };
    // Segment 6: right corner
    pathCells[i++] = { 14, 7 };
    // Segment 7: row 8, cols 14 down to 9
    for (int c = 14; c >= 9; c--) pathCells[i++] = { c, 8 };
    // Segment 8: column 8, rows 9 to 14
    for (int r = 9; r <= 14; r++) pathCells[i++] = { 8, r };
    // Segment 9: bottom corner
    pathCells[i++] = { 7, 14 };
    // Segment 10: column 6, rows 14 down to 9
    for (int r = 14; r >= 9; r--) pathCells[i++] = { 6, r };
    // Segment 11: row 8, cols 5 down to 0
    for (int c = 5; c >= 0; c--) pathCells[i++] = { c, 8 };
    // Segment 12: left corner
    pathCells[i++] = { 0, 7 };
    // Segment 13: back to start of row 6
    pathCells[i++] = { 0, 6 };
    // Total: exactly 52 cells
}

bool Board::isSafePathIndex(int index) const {
    for (int p = 0; p < 4; p++) {
        if (index == entryIndex[p]) return true;
        if (index == (entryIndex[p] + 8) % 52) return true;
    }
    return false;
}

void Board::drawYard(SDL_Renderer* renderer, int col, int row, const Color& color) const {
    for (int r = 0; r < 6; r++)
        for (int c = 0; c < 6; c++)
            GfxUtils::drawCellRect(renderer, (col + c) * CELL, (row + r) * CELL, CELL, CELL, color, lineColor);
}

void Board::drawStar(SDL_Renderer* renderer, int col, int row) const {
    float cx = col * CELL + CELL / 2.0f;
    float cy = row * CELL + CELL / 2.0f;
    float r = CELL * 0.28f;

    SDL_SetRenderDrawColor(renderer, safeStarColor.r, safeStarColor.g, safeStarColor.b, 255);
    const int points = 5;
    for (int i = 0; i < points * 2; i++) {
        float rad = (i % 2 == 0) ? r : r * 0.45f;
        float a1 = (float)i / (points * 2) * 2 * SDL_PI_F - SDL_PI_F / 2;
        SDL_RenderLine(renderer, cx, cy, cx + rad * SDL_cosf(a1), cy + rad * SDL_sinf(a1));
    }
    SDL_FRect marker = { cx - r * 0.5f, cy - r * 0.5f, r, r };
    SDL_RenderFillRect(renderer, &marker);
}

void Board::draw(SDL_Renderer* renderer) const {
    // Base grid background
    for (int r = 0; r < GRID; r++)
        for (int c = 0; c < GRID; c++)
            GfxUtils::drawCellRect(renderer, c * CELL, r * CELL, CELL, CELL, bgColor, lineColor);

    // 4 yards
    drawYard(renderer, yardOrigin[0].col, yardOrigin[0].row, playerColor[0]); // Red
    drawYard(renderer, yardOrigin[1].col, yardOrigin[1].row, playerColor[1]); // Green
    drawYard(renderer, yardOrigin[3].col, yardOrigin[3].row, playerColor[3]); // Teal (bottom-left)
    drawYard(renderer, yardOrigin[2].col, yardOrigin[2].row, playerColor[2]); // Yellow (bottom-right)

    // Cross arms (the shared path track)
    for (int r = 0; r < GRID; r++)
        for (int c = 6; c <= 8; c++)
            GfxUtils::drawCellRect(renderer, c * CELL, r * CELL, CELL, CELL, whiteColor, lineColor);

    for (int r = 6; r <= 8; r++)
        for (int c = 0; c < GRID; c++)
            GfxUtils::drawCellRect(renderer, c * CELL, r * CELL, CELL, CELL, whiteColor, lineColor);

    // Home columns (colored strips leading into the center)
    for (int c = 1; c <= 5; c++)
        GfxUtils::drawCellRect(renderer, c * CELL, 7 * CELL, CELL, CELL, playerColor[0], lineColor); // Red
    for (int r = 1; r <= 5; r++)
        GfxUtils::drawCellRect(renderer, 7 * CELL, r * CELL, CELL, CELL, playerColor[1], lineColor); // Green
    for (int c = 9; c <= 13; c++)
        GfxUtils::drawCellRect(renderer, c * CELL, 7 * CELL, CELL, CELL, playerColor[2], lineColor); // Yellow
    for (int r = 9; r <= 13; r++)
        GfxUtils::drawCellRect(renderer, 7 * CELL, r * CELL, CELL, CELL, playerColor[3], lineColor); // Teal

    // Center home triangle area (kept as a plain square for now)
    for (int r = 6; r <= 8; r++)
        for (int c = 6; c <= 8; c++)
            GfxUtils::drawCellRect(renderer, c * CELL, r * CELL, CELL, CELL, centerColor, lineColor);

    // Safe-cell stars on the shared path
    for (int idx = 0; idx < 52; idx++) {
        if (isSafePathIndex(idx)) {
            drawStar(renderer, pathCells[idx].col, pathCells[idx].row);
        }
    }
}
