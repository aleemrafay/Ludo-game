# Ludo - SDL3

A classic Ludo board game built from scratch in **C++** using **SDL3**, developed in Visual Studio.

## Features (in progress)

- [x] 15x15 board rendering (4 yards, cross path, home columns)
- [x] 52-cell path indexing system (programmatically generated, verified)
- [x] Token/coin system (4 tokens per player, 4 players)
- [x] Mouse-based token selection and movement
- [x] Dice with visual pip rendering and roll logic
- [x] Turn-based system (2-4 players, clockwise order)
- [x] Full movement logic:
  - Yard → Path (requires rolling a 6)
  - Path movement with per-player offset handling
  - Path → Home Stretch entry detection
  - Exact-roll-to-finish rule (no overshoot)
- [x] Kill detection (landing on an opponent sends their token back to yard)
- [x] Safe cell handling (start cells)
- [ ] Star/safe square visuals
- [ ] Center home triangle (4-color split)
- [ ] Win condition / game over screen
- [ ] Three-sixes-forfeit rule

## Tech Stack

- **Language:** C++
- **Graphics:** SDL3
- **IDE:** Visual Studio

## How It Works

- The board is represented as a 15x15 grid (40px per cell).
- The shared 52-cell outer path is generated programmatically (not hardcoded) to avoid indexing mistakes, with each player entering the loop at a fixed offset (`playerIndex * 13`).
- Each player has a private 6-cell "home stretch" column that only their own tokens can travel through, leading to the center.
- Game state (turn, dice, selection, positions) is all tracked in plain C++ structs — no external state management libraries.

## Building

```bash
g++ main_sdl3.cpp -o ludo -lSDL3
./ludo
```

Or open in Visual Studio with the SDL3 development libraries linked.

## Status

Actively in development — built as a personal learning project to strengthen C++ fundamentals, game logic, and SDL rendering.