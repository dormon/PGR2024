cmake_minimum_required(VERSION 3.16)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1 ) 

project(pgr2024)

option(SDL_SHARED "" OFF)
option(SDL_STATIC "" ON)
add_subdirectory(libs/SDL-release-2.30.8)
add_subdirectory(libs/geGL)

add_custom_target(run ./${PROJECT_NAME})

add_executable(${PROJECT_NAME} src/main.cpp src/timer.hpp)

target_link_libraries(${PROJECT_NAME} PUBLIC SDL2::SDL2 SDL2::SDL2main geGL::geGL)

target_include_directories(${PROJECT_NAME} PUBLIC src)
