include(FetchContent)

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

message(STATUS "Embree URL: ${EMBREE_URL}")


# Download the prebuilt Embree package.
set(EMBREE_ROOT_DIR
    "${CMAKE_BINARY_DIR}/_deps/embree"
)

set(EMBREE_ARCHIVE_PATH
    "${CMAKE_BINARY_DIR}/_deps/${EMBREE_ARCHIVE}"
)

file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/_deps")

if(NOT EXISTS "${EMBREE_ARCHIVE_PATH}")
    file(DOWNLOAD
        "${EMBREE_URL}"
        "${EMBREE_ARCHIVE_PATH}"
#        SHOW_PROGRESS
        STATUS EMBREE_DOWNLOAD_STATUS
        TLS_VERIFY ON
    )

    list(GET EMBREE_DOWNLOAD_STATUS 0 EMBREE_DOWNLOAD_CODE)

    if(NOT EMBREE_DOWNLOAD_CODE EQUAL 0)
        list(GET EMBREE_DOWNLOAD_STATUS 1 EMBREE_DOWNLOAD_MESSAGE)

        message(FATAL_ERROR
            "Failed to download Embree: ${EMBREE_DOWNLOAD_MESSAGE}"
        )
    endif()
endif()


# Unzip embree
if(NOT EXISTS "${EMBREE_ROOT_DIR}")
    file(MAKE_DIRECTORY "${EMBREE_ROOT_DIR}")

    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E tar xf "${EMBREE_ARCHIVE_PATH}"
        WORKING_DIRECTORY "${EMBREE_ROOT_DIR}"
        RESULT_VARIABLE EMBREE_EXTRACT_RESULT
    )

    if(NOT EMBREE_EXTRACT_RESULT EQUAL 0)
        message(FATAL_ERROR "Failed to extract Embree")
    endif()
endif()

file(
    GLOB_RECURSE
    EMBREE_CONFIG
    "${EMBREE_ROOT_DIR}/*/embree-config.cmake"
)

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
