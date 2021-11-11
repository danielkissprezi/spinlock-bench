cmake_minimum_required(VERSION 3.21)
project('spin_bench' CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# benchmark options
set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_WERROR OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_EXCEPTIONS OFF CACHE BOOL "")
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "")
set(CMAKE_CXX_STANDARD 17 CACHE STRING "")
# There are pedantic warnings on Windows and apparently no way to turn them off...
set(HAVE_CXX_FLAG_PEDANTIC_ERRORS 0 CACHE BOOL "")
set(HAVE_CXX_FLAG_PEDANTIC 0 CACHE BOOL "")
add_subdirectory(subprojects/benchmark)

add_executable(spin_bench main.cpp)
target_link_libraries(spin_bench PRIVATE benchmark::benchmark benchmark::benchmark_main)
set_property(TARGET spin_bench PROPERTY CXX_STANDARD 17)