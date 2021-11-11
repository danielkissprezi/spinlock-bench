cmake_minimum_required(VERSION 3.21)
project('spin_bench' CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "")
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
add_subdirectory(subprojects/benchmark)

add_executable(spin_bench main.cpp)
target_link_libraries(spin_bench PRIVATE benchmark::benchmark)
set_property(TARGET spin_bench PROPERTY CXX_STANDARD 17)