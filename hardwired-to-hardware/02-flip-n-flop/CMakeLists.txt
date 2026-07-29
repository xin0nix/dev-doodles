cmake_minimum_required(VERSION 3.20)

project(doodle LANGUAGES C CXX)

set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

find_package(verilator REQUIRED)

add_executable(testbench
    tests/main.cpp
)

target_compile_options(testbench PRIVATE -stdlib=libc++)
target_link_options(testbench PRIVATE -stdlib=libc++)

verilate(
    testbench
    TRACE
    SOURCES
        rtl/doodle.v
)

