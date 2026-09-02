cmake_minimum_required(VERSION 3.20)

project(pyembree2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(EMBREE_VERSION "4.4.1")

# Select the prebuilt Embree package.
if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_ARCHIVE
            "embree-${EMBREE_VERSION}.arm64.macosx.zip"
        )
    else()
        set(EMBREE_ARCHIVE
            "embree-${EMBREE_VERSION}.x86_64.macosx.zip"
        )
    endif()

elseif(WIN32)

    set(EMBREE_ARCHIVE
        "embree-${EMBREE_VERSION}.x64.windows.zip"
    )

elseif(UNIX)

    set(EMBREE_ARCHIVE
        "embree-${EMBREE_VERSION}.x86_64.linux.tar.gz"
    )

else()

    message(FATAL_ERROR "Unsupported platform")

endif()

set(EMBREE_URL
    "https://github.com/RenderKit/embree/releases/download/v${EMBREE_VERSION}/${EMBREE_ARCHIVE}"
)

message(STATUS "Embree archive: ${EMBREE_ARCHIVE}")
message(STATUS "Embree URL: ${EMBREE_URL}")

# Download the prebuilt Embree package.
set(EMBREE_ROOT
    "${CMAKE_BINARY_DIR}/_deps/embree"
)

set(EMBREE_ARCHIVE_PATH
    "${CMAKE_BINARY_DIR}/_deps/${EMBREE_ARCHIVE}"
)

file(DOWNLOAD
    "${EMBREE_URL}"
    "${EMBREE_ARCHIVE_PATH}"
    SHOW_PROGRESS
    STATUS EMBREE_DOWNLOAD_STATUS
    TLS_VERIFY ON
)

list(GET EMBREE_DOWNLOAD_STATUS 0 EMBREE_DOWNLOAD_CODE)

if(NOT EMBREE_DOWNLOAD_CODE EQUAL 0)
    list(GET EMBREE_DOWNLOAD_STATUS 1 EMBREE_DOWNLOAD_MESSAGE)

    message(
        FATAL_ERROR
        "Failed to download Embree: ${EMBREE_DOWNLOAD_MESSAGE}"
    )
endif()

file(MAKE_DIRECTORY "${EMBREE_ROOT}")

execute_process(
    COMMAND "${CMAKE_COMMAND}" -E tar xf "${EMBREE_ARCHIVE_PATH}"
    WORKING_DIRECTORY "${EMBREE_ROOT}"
    RESULT_VARIABLE EMBREE_EXTRACT_RESULT
)

if(NOT EMBREE_EXTRACT_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to extract Embree")
endif()

file(
    GLOB_RECURSE
    EMBREE_CONFIG
    "${EMBREE_ROOT}/*/embree-config.cmake"
)

message(STATUS "Embree config candidates:")
foreach(CONFIG_FILE IN LISTS EMBREE_CONFIG)
    message(STATUS "  ${CONFIG_FILE}")
endforeach()

if(NOT EMBREE_CONFIG)
    message(FATAL_ERROR
        "Could not find Embree CMake package after extraction"
    )
endif()

list(GET EMBREE_CONFIG 0 EMBREE_CONFIG)

get_filename_component(
    EMBREE_CMAKE_DIR
    "${EMBREE_CONFIG}"
    DIRECTORY
)

message(STATUS "Embree CMake directory: ${EMBREE_CMAKE_DIR}")

list(PREPEND CMAKE_PREFIX_PATH "${EMBREE_CMAKE_DIR}")

find_package(embree 4 CONFIG REQUIRED)
find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

pybind11_add_module(
    pyembree2
    src/pyembree2/singed_distance.cpp
    src/pyembree2/ray_tracing.cpp
)

target_link_libraries(
    pyembree2
    PRIVATE
    embree
)

install(
    TARGETS pyembree2
    LIBRARY DESTINATION pyembree2
    RUNTIME DESTINATION pyembree2
)
