#pragma once
#include <SDL3/SDL.h>
#include <array>
#include <vector>
#include <utility>
#include "Board.h"
#include "Player.h"
#include "Dice.h"
#include "TokenAnimator.h"

// Game: the top-level orchestrator. Owns the SDL window/renderer, the Board,
// all 4 Players (each owning their own Tokens), the Dice, and the current
// animation. Responsible for the main loop, input handling, and tying
// together all the other classes' behavior each frame.
class Game {
public:
    Game();
    ~Game();

    // Initializes SDL, window, and renderer. Returns false on failure.
    bool init();

    // Runs the main loop until the window is closed. Blocking call.
    void run();

private:
    static const int DICE_PANEL_HEIGHT = 100;
    static const int WINDOW_W = Board::BOARD_PIXELS;
    static const int WINDOW_H = Board::BOARD_PIXELS + DICE_PANEL_HEIGHT;

    SDL_Window* window;
    SDL_Renderer* renderer;
    bool running;

    Board board;
    std::array<Player, 4> players;
    Dice dice;
    TokenAnimator animator;

    int currentPlayerIndex;
    int selectedPlayer, selectedToken;
    bool pendingExtraTurn;

    // ---- Per-frame steps ----
    void handleEvents();
    void handleMouseDown(float mx, float my);
    void update(float dt);
    void render();

    // ---- Turn / move logic ----
    void nextTurn();
    void onDiceRolled();
    void tryStartMoveForClickedToken(int playerIdx, int tokenIdx);
    void finalizeCurrentAnimation();
    void handleCaptures(Token& movedToken);

    // ---- Token picking / stacking helpers ----
    bool hitTestOwnTokens(float mx, float my, int& outTokenIndex) const;
    void getTokensAtCell(int col, int row, std::vector<std::pair<int,int>>& outList) const;
    static void getStackOffset(int indexInStack, int countInStack, float& offX, float& offY);

    // ---- Rendering helpers ----
    void drawTokens();
    void drawTurnIndicator();
    void drawStackCountBadges();

    static const char* playerNameFor(int index);
};
