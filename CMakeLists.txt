cmake_minimum_required(VERSION 3.20)

project(pyembree2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
endif()

set(EMBREE_VERSION "4.4.1")
include(cmake/fetch-embree.cmake)

find_package(embree 4 CONFIG REQUIRED)
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

# Bundle Embree runtime
if(APPLE)
    set(EMBREE_RUNTIME_GLOB "${EMBREE_ROOT_DIR}/lib/*.dylib")
elseif(WIN32)
    set(EMBREE_RUNTIME_GLOB "${EMBREE_ROOT_DIR}/bin/*.dll")
elseif(UNIX)
    set(EMBREE_RUNTIME_GLOB "${EMBREE_ROOT_DIR}/lib/*.so*")
endif()

file(GLOB EMBREE_RUNTIME_LIBRARIES "${EMBREE_RUNTIME_GLOB}")
if(NOT EMBREE_RUNTIME_LIBRARIES)
    message(FATAL_ERROR "Could not find Embree runtime libraries")
endif()
install(FILES ${EMBREE_RUNTIME_LIBRARIES} DESTINATION pyembree2)

# Runtime search path
if(APPLE)
    set_target_properties(
        pyembree2
        PROPERTIES
        INSTALL_RPATH "@loader_path"
        INSTALL_RPATH_USE_LINK_PATH FALSE
    )
elseif(UNIX)
    set_target_properties(
        pyembree2
        PROPERTIES
        INSTALL_RPATH "$ORIGIN"
        INSTALL_RPATH_USE_LINK_PATH FALSE
    )
endif()
# Windows: no RPATH needed, DLLs are found next to the extension automatically.
