# Gomoku C++ Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a cross-platform C++17 console Gomoku course-design project with menus, login, rules/help screens, gameplay, undo, replay, win detection, tests, and GitHub-ready documentation.

**Architecture:** Keep game rules independent from terminal UI so the core can be tested without interactive input. Use CMake to build one playable executable and one lightweight test executable.

**Tech Stack:** C++17, CMake, C++ standard library only.

---

## File Map

- Create `CMakeLists.txt`: CMake targets for app and tests.
- Create `include/Gomoku/Types.h`: shared enums and move records.
- Create `include/Gomoku/Board.h` and `src/Board.cpp`: board state, move validation, undo, win detection, rendering.
- Create `include/Gomoku/Player.h` and `src/Player.cpp`: player model.
- Create `include/Gomoku/Replay.h` and `src/Replay.cpp`: move history and replay snapshots.
- Create `include/Gomoku/UI.h` and `src/UI.cpp`: console menu and display helpers.
- Create `include/Gomoku/Game.h` and `src/Game.cpp`: high-level game loop.
- Create `src/main.cpp`: application entry.
- Create `tests/test_core.cpp`: no-framework tests for board and replay behavior.
- Create `README.md`: build/run instructions for macOS and Windows.

## Tasks

### Task 1: Project Skeleton and Failing Tests

- [ ] Create headers for public APIs.
- [ ] Create `tests/test_core.cpp` with assertions for win detection, invalid moves, undo, and replay.
- [ ] Run `cmake -S . -B build && cmake --build build && ./build/gomoku_tests`.
- [ ] Expected result before implementation: compilation fails because APIs are declared but not defined.

### Task 2: Core Implementation

- [ ] Implement `Types`, `Board`, `Player`, and `Replay`.
- [ ] Run `cmake --build build && ./build/gomoku_tests`.
- [ ] Expected result: all core tests pass.

### Task 3: Console UI and Game Flow

- [ ] Implement `UI`, `Game`, and `main`.
- [ ] Run `cmake --build build`.
- [ ] Expected result: `gomoku` executable builds.

### Task 4: Documentation and Cross-Platform Notes

- [ ] Write `README.md` with macOS and Windows commands.
- [ ] Include usage examples for menu choices, move input, undo, quit, and replay.
- [ ] Run tests again.

### Task 5: Git and GitHub

- [ ] Commit the completed project locally.
- [ ] Create or connect a GitHub repository named `gomoku-cpp-course-design`.
- [ ] Push the `main` branch.
