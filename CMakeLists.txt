cmake_minimum_required(VERSION 4.0)

project(pyembree2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

include(FetchContent)

set(EMBREE_TASKING_SYSTEM "INTERNAL" CACHE STRING "" FORCE)
set(EMBREE_ISPC_SUPPORT OFF CACHE BOOL "" FORCE)
set(EMBREE_TUTORIALS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    embree
    GIT_REPOSITORY https://github.com/RenderKit/embree.git
    GIT_TAG v4.4.0
)

FetchContent_MakeAvailable(embree)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(
    pyembree2
    src/pyembree2/signed_distance.cpp
    src/pyembree2/ray_tracing.cpp
)

target_link_libraries(pyembree2 PRIVATE embree)

install(
    TARGETS pyembree2
    LIBRARY DESTINATION pyembree2
    RUNTIME DESTINATION pyembree2
)

install(
    TARGETS embree
    RUNTIME DESTINATION pyembree2
    LIBRARY DESTINATION pyembree2
    ARCHIVE DESTINATION pyembree2
)

if(APPLE)
    set_target_properties(
        pyembree2
        PROPERTIES
            INSTALL_RPATH "@loader_path"
            BUILD_WITH_INSTALL_RPATH OFF
    )
elseif(UNIX)
    set_target_properties(
        pyembree2
        PROPERTIES
            INSTALL_RPATH "$ORIGIN"
            BUILD_WITH_INSTALL_RPATH OFF
    )
endif()
