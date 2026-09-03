include(FetchContent)

# --- Select archive for this platform ---
if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.arm64.macosx.zip")
    else()
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.macosx.zip")
    endif()
elseif(WIN32)
    set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x64.windows.zip")
elseif(UNIX)
    set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.linux.tar.gz")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(EMBREE_URL "https://github.com/RenderKit/embree/releases/download/v${EMBREE_VERSION}/${EMBREE_ARCHIVE}")
message(STATUS "Embree URL: ${EMBREE_URL}")

# Shared cache dir
if(DEFINED ENV{EMBREE_CACHE_DIR})
    set(FETCHCONTENT_BASE_DIR "$ENV{EMBREE_CACHE_DIR}")
else()
    set(FETCHCONTENT_BASE_DIR "$ENV{HOME}/.cache/pyembree2/_deps")
endif()

FetchContent_Declare(
    embree_prebuilt
    URL "${EMBREE_URL}"
    # URL_HASH SHA256=<fill in per-platform hash>
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(embree_prebuilt)

set(EMBREE_ROOT_DIR "${embree_prebuilt_SOURCE_DIR}")

# Embree CMake config
file(GLOB_RECURSE EMBREE_CONFIG "${EMBREE_ROOT_DIR}/*/embree-config.cmake")
if(NOT EMBREE_CONFIG)
    message(FATAL_ERROR "Could not find Embree CMake package after extraction")
endif()
list(GET EMBREE_CONFIG 0 EMBREE_CONFIG)
get_filename_component(EMBREE_CMAKE_DIR "${EMBREE_CONFIG}" DIRECTORY)

message(STATUS "Embree CMake directory: ${EMBREE_CMAKE_DIR}")
list(PREPEND CMAKE_PREFIX_PATH "${EMBREE_CMAKE_DIR}")
