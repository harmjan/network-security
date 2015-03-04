project(BGPExtract CXX)
cmake_minimum_required(VERSION 3.0.0)

set(CMAKE_CXX_FLAGS "-std=c++0x -Wall -Wextra -g")

add_executable(bgp-extract data.cpp import.cpp main.cpp)
