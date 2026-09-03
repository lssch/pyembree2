include(FetchContent)

if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.arm64.macosx.zip")
    else()
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.macosx.zip")
    endif()
elseif(WIN32)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_BUILD_FROM_SOURCE TRUE)
    else()
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x64.windows.zip")
    endif()
elseif(UNIX)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_BUILD_FROM_SOURCE TRUE)
    else()
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.linux.tar.gz")
    endif()
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

# Shared cache dir - applies to both source and prebuilt fetches
if(DEFINED ENV{EMBREE_CACHE_DIR})
    set(FETCHCONTENT_BASE_DIR "$ENV{EMBREE_CACHE_DIR}")
else()
    set(FETCHCONTENT_BASE_DIR "$ENV{HOME}/.cache/pyembree2/_deps")
endif()

if(EMBREE_BUILD_FROM_SOURCE)
    message(STATUS "No prebuilt Embree for this platform; building from source")

    set(EMBREE_TASKING_SYSTEM "INTERNAL" CACHE STRING "" FORCE)  # avoid needing TBB too
    set(EMBREE_ISPC_SUPPORT OFF CACHE BOOL "" FORCE)             # avoid needing an ISPC compiler
    set(EMBREE_TUTORIALS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        embree_prebuilt
        GIT_REPOSITORY https://github.com/RenderKit/embree.git
        GIT_TAG "v${EMBREE_VERSION}"
    )
    FetchContent_MakeAvailable(embree_prebuilt)

    set(EMBREE_ROOT_DIR "${embree_prebuilt_SOURCE_DIR}")
else()
    set(EMBREE_URL "https://github.com/RenderKit/embree/releases/download/v${EMBREE_VERSION}/${EMBREE_ARCHIVE}")
    message(STATUS "Embree URL: ${EMBREE_URL}")

    FetchContent_Declare(
        embree_prebuilt
        URL "${EMBREE_URL}"
        # URL_HASH SHA256=<fill in per-platform hash>
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(embree_prebuilt)

    set(EMBREE_ROOT_DIR "${embree_prebuilt_SOURCE_DIR}")

    # Embree CMake config
    set(EMBREE_CMAKE_DIR "${EMBREE_ROOT_DIR}/lib/cmake/embree-${EMBREE_VERSION}")
    if(NOT EXISTS "${EMBREE_CMAKE_DIR}/embree-config.cmake")
        set(EMBREE_CMAKE_DIR "${EMBREE_ROOT_DIR}/lib64/cmake/embree-${EMBREE_VERSION}")
    endif()
    if(NOT EXISTS "${EMBREE_CMAKE_DIR}/embree-config.cmake")
        file(GLOB_RECURSE EMBREE_CONFIG "${EMBREE_ROOT_DIR}/*/embree-config.cmake")
        if(NOT EMBREE_CONFIG)
            message(FATAL_ERROR "Could not find Embree CMake package after extraction")
        endif()
        list(GET EMBREE_CONFIG 0 EMBREE_CONFIG)
        get_filename_component(EMBREE_CMAKE_DIR "${EMBREE_CONFIG}" DIRECTORY)
    endif()

    message(STATUS "Embree CMake directory: ${EMBREE_CMAKE_DIR}")
    list(PREPEND CMAKE_PREFIX_PATH "${EMBREE_CMAKE_DIR}")
endif()

find_package(embree 4 CONFIG REQUIRED)
