cmake_minimum_required(VERSION 3.16)

project(GomokuCourseDesign LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_library(gomoku_core STATIC
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
    src/boardwidget.cpp
)

target_link_libraries(gomoku PRIVATE gomoku_core Qt6::Widgets)

add_executable(gomoku_tests
    tests/test_core.cpp
)

target_link_libraries(gomoku_tests PRIVATE gomoku_core)
