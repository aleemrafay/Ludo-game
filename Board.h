#pragma once
#include <SDL3/SDL.h>
#include "GfxUtils.h"

// A single (col,row) cell coordinate on the 15x15 grid.
struct GridPoint {
    int col, row;
};

// Board: owns all the static layout data for a Ludo board - the 52-cell shared
// path, each player's home stretch, yard positions, and safe-cell rules - plus
// the logic/rendering that only depends on that layout (not on any Token state).
//
// There is only ever one board layout, but it's modeled as a regular class
// (not all-static) so it can be constructed once in Game and passed around by
// reference, keeping the design consistent with the rest of the project and
// leaving room for future variants (different board sizes, rule sets, etc.).
class Board {
public:
    static const int CELL = 40;
    static const int GRID = 15;
    static const int BOARD_PIXELS = CELL * GRID;

    Board();

    // Build the 52-cell shared path (called once from the constructor).
    void buildPath();

    // Shared-path accessors
    const GridPoint& pathCellAt(int index) const { return pathCells[index]; }
    int entryIndexFor(int playerIndex) const { return entryIndex[playerIndex]; }
    bool isSafePathIndex(int index) const;

    // Home stretch accessor: cell `stretchIndex` (0-4) of `playerIndex`'s home column.
    const GridPoint& homeStretchCellAt(int playerIndex, int stretchIndex) const {
        return homeStretch[playerIndex][stretchIndex];
    }

    // Yard (starting area) top-left origin for a player, in grid cells.
    const GridPoint& yardOriginFor(int playerIndex) const { return yardOrigin[playerIndex]; }

    // Number of steps a token travels on the shared path before turning into its home stretch.
    static const int STEPS_ON_SHARED_PATH = 51;

    // Draws the full static board (yards, cross arms, home columns, center, safe-cell stars).
    void draw(SDL_Renderer* renderer) const;

private:
    GridPoint pathCells[52];
    int entryIndex[4] = { 0, 13, 26, 39 }; // Red, Green, Yellow, Teal
    GridPoint homeStretch[4][5] = {
        { {1,7},{2,7},{3,7},{4,7},{5,7} },     // Red
        { {7,1},{7,2},{7,3},{7,4},{7,5} },     // Green
        { {13,7},{12,7},{11,7},{10,7},{9,7} }, // Yellow
        { {7,13},{7,12},{7,11},{7,10},{7,9} }  // Teal
    };
    GridPoint yardOrigin[4] = { {0,0}, {9,0}, {9,9}, {0,9} };

    Color bgColor      { 245, 245, 240, 255 };
    Color whiteColor   { 255, 255, 255, 255 };
    Color centerColor  { 220, 220, 220, 255 };
    Color lineColor    { 30, 30, 30, 255 };
    Color safeStarColor{ 170, 170, 170, 255 };
    Color playerColor[4] {
        { 200, 40, 50, 255 },   // Red
        { 60, 140, 70, 255 },   // Green
        { 220, 160, 40, 255 },  // Yellow
        { 50, 90, 100, 255 }    // Teal
    };

    void drawYard(SDL_Renderer* renderer, int col, int row, const Color& color) const;
    void drawStar(SDL_Renderer* renderer, int col, int row) const;

public:
    // Exposed so Token/Player/Game can color things consistently with the board.
    const Color& colorFor(int playerIndex) const { return playerColor[playerIndex]; }
};
