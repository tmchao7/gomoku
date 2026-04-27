cmake_minimum_required(VERSION 3.16)

project(GomokuCourseDesign LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(gomoku_core
    src/Types.cpp
    src/Board.cpp
    src/Player.cpp
    src/Replay.cpp
)

target_include_directories(gomoku_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_executable(gomoku
    src/main.cpp
    src/Game.cpp
    src/UI.cpp
)

target_link_libraries(gomoku PRIVATE gomoku_core)

add_executable(gomoku_tests
    tests/test_core.cpp
)

target_link_libraries(gomoku_tests PRIVATE gomoku_core)
