cmake_minimum_required(VERSION 3.1)
project(aoc)

set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
add_executable(twosum
  twosum.cpp)
add_executable(day2
  day2.cpp)
add_executable(day3
  day3.cpp)
