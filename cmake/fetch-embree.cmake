include(FetchContent)

if(APPLE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.arm64.macosx.zip")
    else()
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.macosx.zip")
    endif()
elseif(WIN32 AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x64.windows.zip")
elseif(UNIX AND CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
    set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.linux.tar.gz")
else()
    message(FATAL_ERROR "Platform ${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR} not supported")
endif()

set(EMBREE_URL
    "https://github.com/RenderKit/embree/releases/download/v${EMBREE_VERSION}/${EMBREE_ARCHIVE}"
)
message(STATUS "Downloading prebuilt Embree from ${EMBREE_URL}")
FetchContent_Declare(
    embree
    URL "${EMBREE_URL}"
)
FetchContent_MakeAvailable(embree)

file(GLOB_RECURSE EMBREE_CONFIG_FILES
    "${embree_SOURCE_DIR}/*embree-config.cmake"
    "${embree_SOURCE_DIR}/*embreeConfig.cmake"
)
if(NOT EMBREE_CONFIG_FILES)
    message(FATAL_ERROR
        "Could not locate embree-config.cmake under ${embree_SOURCE_DIR}"
    )
endif()
list(GET EMBREE_CONFIG_FILES 0 EMBREE_CONFIG_FILE)
get_filename_component(embree_DIR "${EMBREE_CONFIG_FILE}" DIRECTORY)

find_package(embree ${EMBREE_VERSION} EXACT CONFIG REQUIRED
    PATHS "${embree_DIR}"
    NO_DEFAULT_PATH
)

file(GLOB EMBREE_RUNTIME_LIBRARY
    "${embree_SOURCE_DIR}/lib/*.dylib"
    "${embree_SOURCE_DIR}/lib/*.so*"
    "${embree_SOURCE_DIR}/bin/*.dll"
)
