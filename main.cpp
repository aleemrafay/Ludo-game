#include <SDL3/SDL.h>
#include <iostream>
#include <cmath>  // fallback for math functions if SDL macros unavailable

// ---------- Grid constants ----------
const int CELL = 40;                  // px per cell
const int GRID = 15;                  // 15x15 board
const int BOARD_SIZE = CELL * GRID;   // 600x600
const int DICE_PANEL_HEIGHT = 100;    // extra space below board for dice
const int WINDOW_W = BOARD_SIZE;
const int WINDOW_H = BOARD_SIZE + DICE_PANEL_HEIGHT;

// Colors (matching your reference image) - r, g, b, a
struct Color { Uint8 r, g, b, a; };
Color RED_C = { 200, 40, 50, 255 };
Color GREEN_C = { 60, 140, 70, 255 };
Color TEAL_C = { 50, 90, 100, 255 };
Color YELLOW_C = { 220, 160, 40, 255 };
Color BG_C = { 245, 245, 240, 255 };
Color WHITE_C = { 255, 255, 255, 255 };
Color CENTER_C = { 220, 220, 220, 255 };
Color LINE_C = { 30, 30, 30, 255 };

// ---------- Turn system ----------
int currentPlayer = 0; // 0=Red, 1=Green, 2=Yellow, 3=Teal — Red starts
bool diceRolled = false; // whether dice has been rolled this turn

// ---------- The 52-cell shared path, clockwise, starting at Red's exit ----------
// Format: {col, row}
struct Point { int col, row; };

Point pathCells[52];

// Build the path programmatically to avoid hand-tracing mistakes.
// Board is 15x15 (0-14). Arms are 3 cells wide (cols/rows 6,7,8).
// Middle line of each arm (row 7 or col 7) is the HOME COLUMN (not shared path).
// So shared path uses only the two outer lines of each arm: index 6 and 8.
void buildPath() {
    int i = 0;

    // --- Segment 1: Red's row (row 6), moving right, cols 1 to 5 ---
    for (int c = 1; c <= 5; c++) pathCells[i++] = { c, 6 };

    // --- Segment 2: turn up column 6, rows 5 down to 0 ---
    for (int r = 5; r >= 0; r--) pathCells[i++] = { 6, r };

    // --- Segment 3: top row 0, col 7 (single cell, the turn corner) ---
    pathCells[i++] = { 7, 0 };

    // --- Segment 4: column 8, rows 0 to 5, moving down ---
    for (int r = 0; r <= 5; r++) pathCells[i++] = { 8, r };

    // --- Segment 5: Green's row (row 6), moving right, cols 9 to 14 ---
    for (int c = 9; c <= 14; c++) pathCells[i++] = { c, 6 };

    // --- Segment 6: col 14, row 7 (turn corner) ---
    pathCells[i++] = { 14, 7 };

    // --- Segment 7: row 8, cols 14 down to 9, moving left ---
    for (int c = 14; c >= 9; c--) pathCells[i++] = { c, 8 };

    // --- Segment 8: column 8, rows 9 to 14, moving down ---
    for (int r = 9; r <= 14; r++) pathCells[i++] = { 8, r };

    // --- Segment 9: row 14, col 7 (turn corner) ---
    pathCells[i++] = { 7, 14 };

    // --- Segment 10: column 6, rows 14 down to 9, moving up ---
    for (int r = 14; r >= 9; r--) pathCells[i++] = { 6, r };

    // --- Segment 11: row 8, cols 5 down to 0, moving left ---
    for (int c = 5; c >= 0; c--) pathCells[i++] = { c, 8 };

    // --- Segment 12: col 0, row 7 (turn corner) ---
    pathCells[i++] = { 0, 7 };

    // --- Segment 13: row 6, cols 0 to 0 (back toward start) ---
    pathCells[i++] = { 0, 6 };

    // Total should be exactly 52
    // (5 + 6 + 1 + 6 + 6 + 1 + 6 + 6 + 1 + 6 + 6 + 1 + 1 = 52)
}

// ---------- Token / Coin system ----------
enum TokenState { TK_YARD, TK_ON_PATH, TK_HOME_STRETCH, TK_FINISHED };

struct Token {
    int playerIndex;   // 0=Red, 1=Green, 2=Yellow, 3=Teal
    int yardSlot;       // which of the 4 yard slots (0-3)
    TokenState state;
    int pathIndex;       // 0-51 when ON_PATH
    int homeStretchIndex; // 0-5 when HOME_STRETCH
};

const int TOKENS_PER_PLAYER = 4;
Token tokens[4][TOKENS_PER_PLAYER]; // [playerIndex][tokenIndex]

// Yard top-left corner (in grid cells) for each player, used to compute token slot positions
Point yardOrigin[4] = {
    {0, 0},   // Red - top-left
    {9, 0},   // Green - top-right
    {9, 9},   // Yellow - bottom-right
    {0, 9}    // Teal - bottom-left
};

// Darker/richer token colors so they're visible against their own yard background
Color tokenColor(int playerIndex) {
    switch (playerIndex) {
    case 0: return Color{ 150, 20, 30, 255 };   // dark red
    case 1: return Color{ 30, 100, 45, 255 };   // dark green
    case 2: return Color{ 180, 130, 15, 255 };  // dark yellow/gold
    case 3: return Color{ 25, 60, 70, 255 };    // dark teal
    }
    return WHITE_C;
}

void initTokens() {
    for (int p = 0; p < 4; p++) {
        for (int t = 0; t < TOKENS_PER_PLAYER; t++) {
            tokens[p][t].playerIndex = p;
            tokens[p][t].yardSlot = t;
            tokens[p][t].state = TK_YARD;
            tokens[p][t].pathIndex = -1;
            tokens[p][t].homeStretchIndex = -1;
        }
    }
}

// Get the pixel center of a token's yard slot (2x2 arrangement inside the 6x6 yard,
// tucked inside a smaller inner square so it looks like the real board)
void getYardSlotCenter(int playerIndex, int slot, float& outX, float& outY) {
    Point origin = yardOrigin[playerIndex];
    // Inner white square sits at cells (origin+1, origin+1) to (origin+4, origin+4) - a 4x4 area
    // We place 4 slots in a 2x2 pattern within that inner area
    int slotCol = slot % 2; // 0 or 1
    int slotRow = slot / 2; // 0 or 1

    float innerStartX = (origin.col + 1.5f) * CELL;
    float innerStartY = (origin.row + 1.5f) * CELL;
    float spacing = CELL * 2.0f;

    outX = innerStartX + slotCol * spacing + CELL;
    outY = innerStartY + slotRow * spacing + CELL;
}

// Draw a filled circle (SDL3 has no built-in circle, so we approximate with points)
void drawCircle(SDL_Renderer* renderer, float cx, float cy, float radius, Color fill) {
    // White ring behind the token (slightly larger) so it pops against same-color backgrounds
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

    // Outline
    SDL_SetRenderDrawColor(renderer, LINE_C.r, LINE_C.g, LINE_C.b, LINE_C.a);
    const int segments = 24;
    for (int i = 0; i < segments; i++) {
        float a1 = (float)i / segments * 2 * SDL_PI_F;
        float a2 = (float)(i + 1) / segments * 2 * SDL_PI_F;
        SDL_RenderLine(renderer,
            cx + radius * SDL_cosf(a1), cy + radius * SDL_sinf(a1),
            cx + radius * SDL_cosf(a2), cy + radius * SDL_sinf(a2));
    }
}

// ---------- Selection state ----------
int selectedPlayer = -1; // -1 = nothing selected
int selectedToken = -1;

// Check if a click at (mx, my) hits any yard token belonging to the CURRENT player.
// Returns true and fills outPlayer/outToken if hit.
bool hitTestTokens(float mx, float my, int& outPlayer, int& outToken) {
    float radius = CELL * 0.32f;
    int p = currentPlayer; // only current player's tokens are selectable
    for (int t = 0; t < TOKENS_PER_PLAYER; t++) {
        Token& tok = tokens[p][t];
        if (tok.state != TK_YARD) continue; // only yard tokens clickable for now

        float cx, cy;
        getYardSlotCenter(p, tok.yardSlot, cx, cy);

        float dx = mx - cx;
        float dy = my - cy;
        float distSq = dx * dx + dy * dy;
        if (distSq <= radius * radius) {
            outPlayer = p;
            outToken = t;
            return true;
        }
    }
    return false;
}

void drawTokens(SDL_Renderer* renderer) {
    float radius = CELL * 0.32f;
    for (int p = 0; p < 4; p++) {
        for (int t = 0; t < TOKENS_PER_PLAYER; t++) {
            Token& tok = tokens[p][t];
            if (tok.state == TK_YARD) {
                float cx, cy;
                getYardSlotCenter(p, tok.yardSlot, cx, cy);

                bool isSelected = (selectedPlayer == p && selectedToken == t);
                if (isSelected) {
                    // Draw a highlight ring behind the token to show selection
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255); // yellow highlight
                    float hlRadius = radius + 6.0f;
                    for (float dy = -hlRadius; dy <= hlRadius; dy++) {
                        float dx = SDL_sqrtf(hlRadius * hlRadius - dy * dy);
                        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
                    }
                }

                drawCircle(renderer, cx, cy, radius, tokenColor(p));
            }
            // ON_PATH / HOME_STRETCH / FINISHED rendering will be added once movement exists
        }
    }
}

// Advance to the next player's turn (clockwise: Red -> Green -> Yellow -> Teal -> Red)
void nextTurn() {
    currentPlayer = (currentPlayer + 1) % 4;
    diceRolled = false; // new player must roll before selecting/moving
    selectedPlayer = -1;
    selectedToken = -1;
    std::cout << "Turn passed. Now player " << currentPlayer << "'s turn." << std::endl;
}

// ---------- Dice ----------
int diceValue = 1;       // current face value (1-6)

// Dice box position (centered in the panel below the board)
const int DICE_BOX_SIZE = 70;
const int DICE_BOX_X = (WINDOW_W - DICE_BOX_SIZE) / 2;
const int DICE_BOX_Y = BOARD_SIZE + (DICE_PANEL_HEIGHT - DICE_BOX_SIZE) / 2;

void rollDice() {
    if (diceRolled) {
        std::cout << "Already rolled this turn - select and move a token first." << std::endl;
        return;
    }
    diceValue = (SDL_rand(6)) + 1; // SDL3 has SDL_rand(n) -> [0, n)
    diceRolled = true;
    std::cout << "Player " << currentPlayer << " rolled: " << diceValue << std::endl;

    // TEMP: until movement logic exists, we just pass turn automatically after a short delay via click.
    // For now: if no token of the current player can conceivably move, auto pass (placeholder rule).
    // We'll wire real move-based turn passing once movement is implemented.
}

// Returns true if click (mx,my) lands inside the dice box
bool hitTestDice(float mx, float my) {
    return mx >= DICE_BOX_X && mx <= DICE_BOX_X + DICE_BOX_SIZE &&
        my >= DICE_BOX_Y && my <= DICE_BOX_Y + DICE_BOX_SIZE;
}

// Draw a single pip (dot) at a normalized position (0-1, 0-1) within the dice box
void drawPip(SDL_Renderer* renderer, float nx, float ny) {
    float cx = DICE_BOX_X + nx * DICE_BOX_SIZE;
    float cy = DICE_BOX_Y + ny * DICE_BOX_SIZE;
    float r = 5.0f;
    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    for (float dy = -r; dy <= r; dy++) {
        float dx = SDL_sqrtf(r * r - dy * dy);
        SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
    }
}

// Pass-turn button (temporary - until movement logic auto-passes turns)
const int PASS_BTN_W = 100;
const int PASS_BTN_H = 40;
const int PASS_BTN_X = WINDOW_W - PASS_BTN_W - 20;
const int PASS_BTN_Y = BOARD_SIZE + (DICE_PANEL_HEIGHT - PASS_BTN_H) / 2;

bool hitTestPassButton(float mx, float my) {
    return mx >= PASS_BTN_X && mx <= PASS_BTN_X + PASS_BTN_W &&
        my >= PASS_BTN_Y && my <= PASS_BTN_Y + PASS_BTN_H;
}

void drawPassButton(SDL_Renderer* renderer) {
    SDL_FRect btn = { (float)PASS_BTN_X, (float)PASS_BTN_Y, (float)PASS_BTN_W, (float)PASS_BTN_H };
    // Only "active" looking if dice has been rolled
    if (diceRolled) {
        SDL_SetRenderDrawColor(renderer, 80, 130, 200, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 190, 190, 190, 255);
    }
    SDL_RenderFillRect(renderer, &btn);
    SDL_SetRenderDrawColor(renderer, LINE_C.r, LINE_C.g, LINE_C.b, LINE_C.a);
    SDL_RenderRect(renderer, &btn);
}

void drawDice(SDL_Renderer* renderer) {
    // Panel background
    SDL_FRect panel = { 0, (float)BOARD_SIZE, (float)WINDOW_W, (float)DICE_PANEL_HEIGHT };
    SDL_SetRenderDrawColor(renderer, 235, 235, 230, 255);
    SDL_RenderFillRect(renderer, &panel);

    // Turn indicator: a colored circle on the left side showing whose turn it is
    Color turnColor = tokenColor(currentPlayer);
    float indCx = 70;
    float indCy = BOARD_SIZE + DICE_PANEL_HEIGHT / 2.0f;
    float indR = 22.0f;
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    for (float dy = -indR - 3; dy <= indR + 3; dy++) {
        float dx = SDL_sqrtf((indR + 3) * (indR + 3) - dy * dy);
        SDL_RenderLine(renderer, indCx - dx, indCy + dy, indCx + dx, indCy + dy);
    }
    SDL_SetRenderDrawColor(renderer, turnColor.r, turnColor.g, turnColor.b, 255);
    for (float dy = -indR; dy <= indR; dy++) {
        float dx = SDL_sqrtf(indR * indR - dy * dy);
        SDL_RenderLine(renderer, indCx - dx, indCy + dy, indCx + dx, indCy + dy);
    }
    SDL_SetRenderDrawColor(renderer, LINE_C.r, LINE_C.g, LINE_C.b, LINE_C.a);
    const int segs = 24;
    for (int i = 0; i < segs; i++) {
        float a1 = (float)i / segs * 2 * SDL_PI_F;
        float a2 = (float)(i + 1) / segs * 2 * SDL_PI_F;
        SDL_RenderLine(renderer,
            indCx + indR * SDL_cosf(a1), indCy + indR * SDL_sinf(a1),
            indCx + indR * SDL_cosf(a2), indCy + indR * SDL_sinf(a2));
    }

    // Dice box
    SDL_FRect box = { (float)DICE_BOX_X, (float)DICE_BOX_Y, (float)DICE_BOX_SIZE, (float)DICE_BOX_SIZE };
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, LINE_C.r, LINE_C.g, LINE_C.b, LINE_C.a);
    SDL_RenderRect(renderer, &box);

    // Pip layout positions (normalized 0-1 within box)
    const float L = 0.22f, M = 0.5f, R = 0.78f; // left/mid/right
    const float T = 0.22f, C = 0.5f, B = 0.78f; // top/center/bottom

    switch (diceValue) {
    case 1:
        drawPip(renderer, M, C);
        break;
    case 2:
        drawPip(renderer, L, T);
        drawPip(renderer, R, B);
        break;
    case 3:
        drawPip(renderer, L, T);
        drawPip(renderer, M, C);
        drawPip(renderer, R, B);
        break;
    case 4:
        drawPip(renderer, L, T);
        drawPip(renderer, R, T);
        drawPip(renderer, L, B);
        drawPip(renderer, R, B);
        break;
    case 5:
        drawPip(renderer, L, T);
        drawPip(renderer, R, T);
        drawPip(renderer, M, C);
        drawPip(renderer, L, B);
        drawPip(renderer, R, B);
        break;
    case 6:
        drawPip(renderer, L, T);
        drawPip(renderer, R, T);
        drawPip(renderer, L, C);
        drawPip(renderer, R, C);
        drawPip(renderer, L, B);
        drawPip(renderer, R, B);
        break;
    }
}

// Draw a single grid cell at (col, row) with given fill color + outline
void drawCell(SDL_Renderer* renderer, int col, int row, Color fill) {
    SDL_FRect rect = { (float)(col * CELL), (float)(row * CELL), (float)CELL, (float)CELL };

    SDL_SetRenderDrawColor(renderer, fill.r, fill.g, fill.b, fill.a);
    SDL_RenderFillRect(renderer, &rect);

    // Outline
    SDL_SetRenderDrawColor(renderer, LINE_C.r, LINE_C.g, LINE_C.b, LINE_C.a);
    SDL_RenderRect(renderer, &rect);
}

// Draw a 6x6 yard block starting at top-left cell (col, row)
void drawYard(SDL_Renderer* renderer, int col, int row, Color color) {
    for (int r = 0; r < 6; r++) {
        for (int c = 0; c < 6; c++) {
            drawCell(renderer, col + c, row + r, color);
        }
    }
}

// Draw a small marker (dot) on a path cell to visually trace the loop order.
// Color shifts gradually from dark to light across the 52 cells so you can SEE the direction of travel.
void drawPathMarkers(SDL_Renderer* renderer) {
    for (int idx = 0; idx < 52; idx++) {
        Point p = pathCells[idx];
        // Gradient: index 0 = dark blue, index 51 = bright blue (shows direction/order)
        Uint8 shade = (Uint8)(50 + (idx * 3)); // 50 -> ~203
        SDL_SetRenderDrawColor(renderer, 20, 20, shade, 255);

        float cx = p.col * CELL + CELL / 2.0f;
        float cy = p.row * CELL + CELL / 2.0f;
        SDL_FRect marker = { cx - 6, cy - 6, 12, 12 };
        SDL_RenderFillRect(renderer, &marker);
    }
}

void drawBoard(SDL_Renderer* renderer) {
    // Base grid
    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            drawCell(renderer, c, r, BG_C);
        }
    }

    // 4 yards
    drawYard(renderer, 0, 0, RED_C);     // top-left
    drawYard(renderer, 9, 0, GREEN_C);   // top-right
    drawYard(renderer, 0, 9, TEAL_C);    // bottom-left
    drawYard(renderer, 9, 9, YELLOW_C);  // bottom-right

    // Cross arms (the path, cols/rows 6-8)
    // Vertical arm
    for (int r = 0; r < GRID; r++) {
        for (int c = 6; c <= 8; c++) {
            drawCell(renderer, c, r, WHITE_C);
        }
    }
    // Horizontal arm
    for (int r = 6; r <= 8; r++) {
        for (int c = 0; c < GRID; c++) {
            drawCell(renderer, c, r, WHITE_C);
        }
    }

    // Home columns (middle row/col of each arm, colored strip leading to center)
    // Red: left arm, row 7, cols 1-5
    for (int c = 1; c <= 5; c++) {
        drawCell(renderer, c, 7, RED_C);
    }
    // Green: top arm, col 7, rows 1-5
    for (int r = 1; r <= 5; r++) {
        drawCell(renderer, 7, r, GREEN_C);
    }
    // Yellow: right arm, row 7, cols 9-13
    for (int c = 9; c <= 13; c++) {
        drawCell(renderer, c, 7, YELLOW_C);
    }
    // Teal: bottom arm, col 7, rows 9-13
    for (int r = 9; r <= 13; r++) {
        drawCell(renderer, 7, r, TEAL_C);
    }

    // Center home area (3x3, cols/rows 6-8) - placeholder, will split into 4 triangles later
    for (int r = 6; r <= 8; r++) {
        for (int c = 6; c <= 8; c++) {
            drawCell(renderer, c, r, CENTER_C);
        }
    }
}

int main(int argc, char* argv[]) {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window* window = SDL_CreateWindow("Ludo - SDL3", WINDOW_W, WINDOW_H, 0);
    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    buildPath();
    initTokens();
    SDL_srand(0); // 0 = seed using performance counter (proper randomness each run)

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;

            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
                float mx = event.button.x;
                float my = event.button.y;

                if (hitTestPassButton(mx, my)) {
                    if (diceRolled) {
                        nextTurn();
                    }
                    else {
                        std::cout << "Roll the dice before passing." << std::endl;
                    }
                    continue;
                }

                if (hitTestDice(mx, my)) {
                    rollDice();
                    continue; // don't also process as a token click
                }

                int clickedPlayer, clickedToken;
                if (diceRolled && hitTestTokens(mx, my, clickedPlayer, clickedToken)) {
                    // Toggle selection: clicking the same token again deselects it
                    if (selectedPlayer == clickedPlayer && selectedToken == clickedToken) {
                        selectedPlayer = -1;
                        selectedToken = -1;
                    }
                    else {
                        selectedPlayer = clickedPlayer;
                        selectedToken = clickedToken;
                    }
                    std::cout << "Selected player " << selectedPlayer << " token " << selectedToken << std::endl;
                }
                else if (!diceRolled) {
                    std::cout << "Roll the dice first!" << std::endl;
                }
                else {
                    // Clicked empty space - deselect
                    selectedPlayer = -1;
                    selectedToken = -1;
                }
            }
        }

        SDL_SetRenderDrawColor(renderer, BG_C.r, BG_C.g, BG_C.b, BG_C.a);
        SDL_RenderClear(renderer);

        drawBoard(renderer);
        drawTokens(renderer);
        drawDice(renderer);
        drawPassButton(renderer);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}