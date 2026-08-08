# 🎲 Ludo Game — C++ / SDL3 (OOP Edition)

A fully playable **Ludo board game** built from scratch in modern C++ using **SDL3**, featuring smooth token animation, full Ludo rules (safe cells, kills, home stretch), and a clean **object-oriented architecture**.

**Language:** C++17 · **Library:** SDL3 · **Platform:** Windows

---

## 📖 Overview

This project started as a single-file SDL3 renderer and was refactored into a proper **multi-file, class-based architecture** — built as part of an OOP coursework project. It implements the complete core Ludo ruleset: rolling dice, moving tokens along a shared 52-cell path, entering home stretches, capturing opponents, and stacking on safe cells.

## ✨ Features

- 🎯 **Auto-movement** — select a token, and it smoothly slides step-by-step to its destination (no manual dragging)
- 🎲 **Full dice logic** — 6 to leave the yard, extra turn on rolling a 6, automatic turn-passing when no legal move exists
- ⭐ **Safe cells** — star-marked cells where tokens can't be captured, matching standard Ludo board layout
- ⚔️ **Kill detection** — landing on an opponent's token sends it back to their yard (unless on a safe cell)
- 🏠 **Home stretch & finish** — tokens peel off the shared path into their own colored home column and finish at the center
- 🟢🔴🟡🔵 **Token stacking visuals** — multiple tokens sharing a cell fan out visibly with a count badge, instead of overlapping invisibly
- 🖱️ **Simple click-to-move controls** — click the dice to roll, click a token to move it

## 🏗️ Architecture

The game is built around clear, single-responsibility classes rather than one large procedural file:

| Class | Responsibility |
|---|---|
| **`Game`** | Top-level orchestrator — owns the SDL window/renderer, runs the main loop, handles input, and manages turn state |
| **`Board`** | Static board layout — the 52-cell shared path, each player's home stretch, yard positions, and safe-cell rules |
| **`Player`** | Owns 4 `Token` objects, and knows its own name/color/index |
| **`Token`** | A single game piece — encapsulates its own state (yard / on-path / home-stretch / finished) and movement logic |
| **`Dice`** | Owns the current roll value, roll logic, and its own rendering |
| **`TokenAnimator`** | Drives the smooth slide animation for a moving token, frame by frame |
| **`GfxUtils`** | Shared static drawing helpers (filled circles, cell rectangles) reused across classes |

**Design principles applied:**
- **Encapsulation** — internal state (e.g. a token's path index) is only ever changed through methods like `applyMove()`, never poked directly
- **Composition** — `Game` *has-a* `Board`, 4 `Player`s, a `Dice`; each `Player` *has-a* 4 `Token`s
- **Separation of concerns** — movement is split into "compute the path" (`buildMovePath`, pure/no side effects) vs. "apply the result" (`applyMove`), so the same logic can drive animation before it drives actual state

## 📂 Project Structure

```
Ludo_game/
├── main.cpp              # Entry point — creates and runs the Game
├── Game.h / Game.cpp      # Main loop, input handling, turn orchestration
├── Board.h / Board.cpp    # Board layout, path data, board rendering
├── Player.h / Player.cpp  # Player identity + owns 4 tokens
├── Token.h / Token.cpp    # Single token's state and movement logic
├── Dice.h / Dice.cpp      # Dice roll state and rendering
├── TokenAnimator.h/.cpp   # Smooth slide-animation state machine
└── GfxUtils.h / .cpp      # Shared SDL drawing primitives
```

## 🛠️ Built With

- **C++17**
- **[SDL3](https://www.libsdl.org/)** — window, rendering, input, and timing

## 🚀 Getting Started

### Prerequisites
- Visual Studio (2022 or later recommended) with the Desktop C++ workload
- SDL3 installed and linked (via [vcpkg](https://github.com/microsoft/vcpkg) is the easiest route)

### Build & Run
1. Clone the repository:
   ```bash
   git clone https://github.com/aleemrafay/Ludo-game.git
   ```
2. Open `Ludo_game.slnx` in Visual Studio.
3. Make sure the platform is set to **x64** and the C++ Language Standard is **ISO C++17**.
4. Build the solution (`Ctrl+Shift+B`).
5. Run (`Ctrl+F5`).

## 🎮 How to Play

1. Click the **dice box** to roll.
2. If you have a legal move, click one of your tokens — it will automatically slide to its new position.
3. Roll a **6** to bring a token out of the yard, or to earn an extra turn.
4. Land on an opponent's token (off a safe/star cell) to send it back to their yard.
5. Get all 4 tokens home to win!

## 🗺️ Roadmap / Possible Improvements

- [ ] On-screen turn indicator (currently console-only)
- [ ] Win screen / game-over state
- [ ] Sound effects for rolls, moves, and captures
- [ ] Configurable number of players (2-player mode)

## 👤 Author

**RafayAleem** ([@aleemrafay](https://github.com/aleemrafay))
BS Artificial Intelligence, University of Central Punjab

---

*Built as a learning project to practice C++ OOP design, SDL3 graphics programming, and game-state management.*