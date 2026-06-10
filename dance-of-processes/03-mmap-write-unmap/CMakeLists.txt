cmake_minimum_required(VERSION 3.12)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)
set(CMAKE_C_COMPILER clang)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(share-mem LANGUAGES C)

add_compile_options(-fdefer-ts)
add_compile_options(
    -Wall
    -Wextra
    -Wpedantic
    -Wconversion
    -Wshadow
    -Wdouble-promotion
    -Wformat=2
)

add_executable(doodle main.c)

