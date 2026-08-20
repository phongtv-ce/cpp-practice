cmake_minimum_required(VERSION 3.14)
project(SnakeGame LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(snake
    Snake.cpp
    game.cpp
)

if(WIN32)
    target_sources(snake PRIVATE console_win32.cpp)
else()
    find_package(Curses REQUIRED)
    target_sources(snake PRIVATE console_ncurses.cpp)
    target_include_directories(snake PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(snake PRIVATE ${CURSES_LIBRARIES})
endif()
