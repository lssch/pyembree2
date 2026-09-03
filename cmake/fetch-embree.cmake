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
    set(EMBREE_BUILD_FROM_SOURCE TRUE)
endif()

if(EMBREE_BUILD_FROM_SOURCE)
    message(STATUS "No prebuilt Embree available; building from source")
    set(EMBREE_TASKING_SYSTEM "INTERNAL" CACHE STRING "" FORCE)
    set(EMBREE_ISPC_SUPPORT OFF CACHE BOOL "" FORCE)
    set(EMBREE_TUTORIALS OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(
        embree
        GIT_REPOSITORY https://github.com/RenderKit/embree.git
        GIT_TAG "v${EMBREE_VERSION}"
    )
    FetchContent_MakeAvailable(embree)
else()
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
endif()
