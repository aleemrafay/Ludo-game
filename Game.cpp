#include "Game.h"
#include <iostream>
#include <cmath>

Game::Game()
    : window(nullptr), renderer(nullptr), running(false),
      players{ Player(0), Player(1), Player(2), Player(3) },
      dice((WINDOW_W - 70) / 2, Board::BOARD_PIXELS + (DICE_PANEL_HEIGHT - 70) / 2, 70),
      currentPlayerIndex(0), selectedPlayer(-1), selectedToken(-1), pendingExtraTurn(false) {
}

Game::~Game() {
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window) SDL_DestroyWindow(window);
    SDL_Quit();
}

const char* Game::playerNameFor(int index) {
    switch (index) {
    case 0: return "Red";
    case 1: return "Green";
    case 2: return "Yellow";
    case 3: return "Teal";
    default: return "Unknown";
    }
}

bool Game::init() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        std::cout << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow("Ludo - SDL3 (OOP)", WINDOW_W, WINDOW_H, 0);
    if (!window) {
        std::cout << "Window creation failed: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return false;
    }

    renderer = SDL_CreateRenderer(window, NULL);
    if (!renderer) {
        std::cout << "Renderer creation failed: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return false;
    }

    SDL_srand(0); // seed using performance counter
    running = true;
    return true;
}

void Game::run() {
    Uint64 lastTicks = SDL_GetTicks();

    while (running) {
        Uint64 nowTicks = SDL_GetTicks();
        float dt = (nowTicks - lastTicks) / 1000.0f;
        lastTicks = nowTicks;

        handleEvents();
        update(dt);
        render();
    }
}

// ---------------- Event handling ----------------

void Game::handleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_EVENT_QUIT) {
            running = false;
        }
        if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
            handleMouseDown(event.button.x, event.button.y);
        }
    }
}

void Game::handleMouseDown(float mx, float my) {
    if (animator.isRunning()) return; // ignore input mid-animation

    if (dice.hitTest(mx, my)) {
        onDiceRolled();
        return;
    }

    int clickedTokenIndex;
    if (dice.hasBeenRolled() && hitTestOwnTokens(mx, my, clickedTokenIndex)) {
        tryStartMoveForClickedToken(currentPlayerIndex, clickedTokenIndex);
    } else if (!dice.hasBeenRolled()) {
        std::cout << "Roll the dice first!" << std::endl;
    }
}

// ---------------- Turn / dice logic ----------------

void Game::onDiceRolled() {
    if (!dice.roll()) {
        std::cout << "Already rolled - select a token to move." << std::endl;
        return;
    }

    std::cout << playerNameFor(currentPlayerIndex) << " rolled: " << dice.getValue() << std::endl;

    if (!players[currentPlayerIndex].hasAnyLegalMove(board, dice.getValue())) {
        std::cout << "No legal moves. Passing turn." << std::endl;
        nextTurn();
    }
}

void Game::nextTurn() {
    currentPlayerIndex = (currentPlayerIndex + 1) % 4;
    dice.resetForNewTurn();
    selectedPlayer = -1;
    selectedToken = -1;
    std::cout << "Turn passed. Now it's " << playerNameFor(currentPlayerIndex) << "'s turn." << std::endl;
}

void Game::tryStartMoveForClickedToken(int playerIdx, int tokenIdx) {
    Token& tok = players[playerIdx].getToken(tokenIdx);
    int diceValue = dice.getValue();

    if (!tok.canMove(board, diceValue)) {
        std::cout << "That token can't move " << diceValue << " steps." << std::endl;
        return;
    }

    selectedPlayer = playerIdx;
    selectedToken = tokenIdx;
    pendingExtraTurn = (diceValue == 6);

    std::vector<GridPoint> movePath = tok.buildMovePath(board, diceValue);

    float startCol, startRow;
    if (tok.getState() == TokenState::YARD) {
        players[playerIdx].getYardSlotCenter(board, tokenIdx, startCol, startRow);
    } else {
        float dummyYardCol = 0, dummyYardRow = 0;
        tok.getGridPos(board, dummyYardCol, dummyYardRow, startCol, startRow);
    }

    animator.start(playerIdx, tokenIdx, movePath, startCol, startRow, 6.0f);

    std::cout << "Auto-moving " << playerNameFor(playerIdx)
              << "'s token " << (tokenIdx + 1)
              << " by " << diceValue << " steps." << std::endl;
}

void Game::finalizeCurrentAnimation() {
    int p = animator.getAnimatingPlayer();
    int t = animator.getAnimatingToken();
    int steps = animator.getStepCount();

    Token& movedToken = players[p].getToken(t);
    movedToken.applyMove(board, steps);

    handleCaptures(movedToken);

    selectedPlayer = -1;
    selectedToken = -1;
    dice.resetForNewTurn();

    if (players[p].hasWon()) {
        std::cout << playerNameFor(p) << " has won the game!" << std::endl;
        // Game continues running so remaining players can finish; no forced exit.
    }

    if (pendingExtraTurn) {
        pendingExtraTurn = false;
        std::cout << "Rolled a 6 - go again!" << std::endl;
        // currentPlayerIndex stays the same; dice already reset so they can roll again.
    } else {
        nextTurn();
    }
}

void Game::handleCaptures(Token& movedToken) {
    if (movedToken.getState() != TokenState::ON_PATH) return;

    int landedIdx = movedToken.getPathIndex();
    if (board.isSafePathIndex(landedIdx)) return;

    int p = movedToken.getPlayerIndex();
    for (int op = 0; op < 4; op++) {
        if (op == p) continue;
        for (int ot = 0; ot < Player::TOKENS_PER_PLAYER; ot++) {
            Token& other = players[op].getToken(ot);
            if (other.getState() == TokenState::ON_PATH && other.getPathIndex() == landedIdx) {
                other.sendToYard();
                std::cout << playerNameFor(p) << " killed " << playerNameFor(op) << "'s token!" << std::endl;
            }
        }
    }
}

// ---------------- Update ----------------

void Game::update(float dt) {
    if (animator.isRunning()) {
        bool finished = animator.update(dt);
        if (finished) {
            finalizeCurrentAnimation();
        }
    }
}

// ---------------- Token picking / stacking ----------------

bool Game::hitTestOwnTokens(float mx, float my, int& outTokenIndex) const {
    float radius = Board::CELL * 0.20f;
    Player& player = const_cast<Player&>(players[currentPlayerIndex]);

    for (int t = 0; t < Player::TOKENS_PER_PLAYER; t++) {
        Token& tok = player.getToken(t);
        if (tok.getState() == TokenState::FINISHED) continue;

        float gc, gr;
        if (tok.getState() == TokenState::YARD) {
            player.getYardSlotCenter(board, t, gc, gr);
        } else {
            float dummy = 0;
            tok.getGridPos(board, dummy, dummy, gc, gr);
        }

        float cx = gc * Board::CELL;
        float cy = gr * Board::CELL;

        if (tok.getState() != TokenState::YARD) {
            std::vector<std::pair<int,int>> stack;
            getTokensAtCell((int)gc, (int)gr, stack);
            int countInStack = (int)stack.size();
            int indexInStack = 0;
            for (int i = 0; i < countInStack; i++) {
                if (stack[i].first == currentPlayerIndex && stack[i].second == t) { indexInStack = i; break; }
            }
            float offX, offY;
            getStackOffset(indexInStack, countInStack, offX, offY);
            cx += offX * Board::CELL;
            cy += offY * Board::CELL;
        }

        float dx = mx - cx;
        float dy = my - cy;
        if (dx * dx + dy * dy <= radius * radius) {
            outTokenIndex = t;
            return true;
        }
    }
    return false;
}

void Game::getTokensAtCell(int col, int row, std::vector<std::pair<int,int>>& outList) const {
    outList.clear();
    for (int p = 0; p < 4; p++) {
        for (int t = 0; t < Player::TOKENS_PER_PLAYER; t++) {
            const Token& tok = players[p].getToken(t);
            if (tok.getState() == TokenState::FINISHED || tok.getState() == TokenState::YARD) continue;
            if (animator.isRunning() && animator.getAnimatingPlayer() == p && animator.getAnimatingToken() == t) continue;

            float dummy = 0, gc, gr;
            tok.getGridPos(board, dummy, dummy, gc, gr);
            if ((int)gc == col && (int)gr == row) {
                outList.push_back({ p, t });
            }
        }
    }
}

void Game::getStackOffset(int indexInStack, int countInStack, float& offX, float& offY) {
    if (countInStack <= 1) { offX = 0; offY = 0; return; }
    if (countInStack == 2) {
        float positions[2][2] = { {-0.18f, 0}, {0.18f, 0} };
        offX = positions[indexInStack][0];
        offY = positions[indexInStack][1];
        return;
    }
    if (countInStack == 3) {
        float positions[3][2] = { {-0.18f,-0.15f}, {0.18f,-0.15f}, {0.0f, 0.18f} };
        offX = positions[indexInStack][0];
        offY = positions[indexInStack][1];
        return;
    }
    int idx = indexInStack % 4;
    float positions[4][2] = { {-0.18f,-0.18f}, {0.18f,-0.18f}, {-0.18f,0.18f}, {0.18f,0.18f} };
    offX = positions[idx][0];
    offY = positions[idx][1];
}

// ---------------- Rendering ----------------

void Game::render() {
    SDL_SetRenderDrawColor(renderer, 245, 245, 240, 255);
    SDL_RenderClear(renderer);

    board.draw(renderer);
    drawTokens();
    drawStackCountBadges();
    drawTurnIndicator();
    dice.draw(renderer);

    SDL_RenderPresent(renderer);
}

void Game::drawTokens() {
    float radius = Board::CELL * 0.28f;

    for (int p = 0; p < 4; p++) {
        for (int t = 0; t < Player::TOKENS_PER_PLAYER; t++) {
            Token& tok = players[p].getToken(t);
            if (tok.getState() == TokenState::FINISHED) continue;

            bool thisIsAnimating = animator.isRunning() &&
                                    animator.getAnimatingPlayer() == p &&
                                    animator.getAnimatingToken() == t;

            float gc, gr;
            if (thisIsAnimating) {
                animator.getInterpolatedPos(gc, gr);
            } else if (tok.getState() == TokenState::YARD) {
                players[p].getYardSlotCenter(board, t, gc, gr);
            } else {
                float dummy = 0;
                tok.getGridPos(board, dummy, dummy, gc, gr);
            }

            float cx = gc * Board::CELL;
            float cy = gr * Board::CELL;

            if (!thisIsAnimating && tok.getState() != TokenState::YARD) {
                std::vector<std::pair<int,int>> stack;
                getTokensAtCell((int)gc, (int)gr, stack);
                int countInStack = (int)stack.size();
                if (countInStack > 1) {
                    int indexInStack = 0;
                    for (int i = 0; i < countInStack; i++) {
                        if (stack[i].first == p && stack[i].second == t) { indexInStack = i; break; }
                    }
                    float offX, offY;
                    getStackOffset(indexInStack, countInStack, offX, offY);
                    cx += offX * Board::CELL;
                    cy += offY * Board::CELL;
                }
            }

            bool isSelected = (selectedPlayer == p && selectedToken == t);
            if (isSelected) {
                Color highlight{ 255, 255, 0, 255 };
                SDL_SetRenderDrawColor(renderer, highlight.r, highlight.g, highlight.b, highlight.a);
                float hlRadius = radius * 0.7f + 6.0f;
                for (float dy = -hlRadius; dy <= hlRadius; dy++) {
                    float dx = SDL_sqrtf(hlRadius * hlRadius - dy * dy);
                    SDL_RenderLine(renderer, cx - dx, cy + dy, cx + dx, cy + dy);
                }
            }

            GfxUtils::drawFilledCircleWithRing(renderer, cx, cy, radius * 0.62f, board.colorFor(p));
        }
    }
}

void Game::drawStackCountBadges() {
    for (int col = 0; col < Board::GRID; col++) {
        for (int row = 0; row < Board::GRID; row++) {
            std::vector<std::pair<int,int>> stack;
            getTokensAtCell(col, row, stack);
            if ((int)stack.size() < 2) continue;

            float cx = (col + 0.5f) * Board::CELL;
            float cy = (row + 0.5f) * Board::CELL;
            float bx = cx + Board::CELL * 0.32f;
            float by = cy - Board::CELL * 0.32f;
            float br = 9.0f;

            Color dark{ 20, 20, 20, 230 };
            GfxUtils::drawFilledCircle(renderer, bx, by, br, dark);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_FRect ring = { bx - br, by - br, br * 2, br * 2 };
            SDL_RenderRect(renderer, &ring);

            int count = (int)stack.size();
            auto pip = [&](float nx, float ny) {
                float px = bx + nx * br;
                float py = by + ny * br;
                SDL_FRect r = { px - 1.6f, py - 1.6f, 3.2f, 3.2f };
                SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
                SDL_RenderFillRect(renderer, &r);
            };
            switch (count) {
            case 2: pip(-0.4f, 0); pip(0.4f, 0); break;
            case 3: pip(-0.4f, -0.3f); pip(0.4f, -0.3f); pip(0, 0.4f); break;
            case 4: pip(-0.4f, -0.4f); pip(0.4f, -0.4f); pip(-0.4f, 0.4f); pip(0.4f, 0.4f); break;
            default:
                for (int i = 0; i < 6 && i < count; i++) {
                    float a = (float)i / 6.0f * 2 * SDL_PI_F;
                    pip(0.45f * SDL_cosf(a), 0.45f * SDL_sinf(a));
                }
                break;
            }
        }
    }
}

void Game::drawTurnIndicator() {
    Color turnColor = board.colorFor(currentPlayerIndex);
    float indCx = 70;
    float indCy = Board::BOARD_PIXELS + DICE_PANEL_HEIGHT / 2.0f;
    float indR = 22.0f;

    Color white{ 255, 255, 255, 255 };
    GfxUtils::drawFilledCircle(renderer, indCx, indCy, indR + 3.0f, white);
    GfxUtils::drawFilledCircle(renderer, indCx, indCy, indR, turnColor);

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    const int segs = 24;
    for (int i = 0; i < segs; i++) {
        float a1 = (float)i / segs * 2 * SDL_PI_F;
        float a2 = (float)(i + 1) / segs * 2 * SDL_PI_F;
        SDL_RenderLine(renderer,
            indCx + indR * SDL_cosf(a1), indCy + indR * SDL_sinf(a1),
            indCx + indR * SDL_cosf(a2), indCy + indR * SDL_sinf(a2));
    }
}
