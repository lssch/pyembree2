cmake_minimum_required(VERSION 4.2)
project(pyembree2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(embree REQUIRED)
find_package(pybind11 REQUIRED)

pybind11_add_module(pyembree2 pyembree2.cpp)
target_link_libraries(pyembree2 PRIVATE embree)
