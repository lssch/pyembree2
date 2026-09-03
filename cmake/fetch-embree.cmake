include(FetchContent)

if(APPLE)
    set(EMBREE_BUILD_FROM_SOURCE TRUE)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.arm64.macosx.zip")
    else()
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.macosx.zip")
    endif()
elseif(WIN32)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x64.windows.zip")
    else()
        set(EMBREE_BUILD_FROM_SOURCE TRUE)
    endif()
elseif(UNIX)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64|amd64")
        set(EMBREE_ARCHIVE "embree-${EMBREE_VERSION}.x86_64.linux.tar.gz")
    else()
        set(EMBREE_BUILD_FROM_SOURCE TRUE)
    endif()
else()
    set(EMBREE_BUILD_FROM_SOURCE TRUE)
endif()

if(EMBREE_BUILD_FROM_SOURCE)
    message(STATUS
        "No prebuilt Embree available for "
        "${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}; "
        "building from source"
    )

    set(EMBREE_TASKING_SYSTEM "INTERNAL" CACHE STRING "" FORCE)
    set(EMBREE_ISPC_SUPPORT OFF CACHE BOOL "" FORCE)
    set(EMBREE_TUTORIALS OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        embree
        GIT_REPOSITORY https://github.com/RenderKit/embree.git
        GIT_TAG "v${EMBREE_VERSION}"
    )
else()
    set(EMBREE_URL
        "https://github.com/RenderKit/embree/releases/download"
        "/v${EMBREE_VERSION}/${EMBREE_ARCHIVE}"
    )

    message(STATUS
        "Prebuilt Embree available for "
        "${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}; "
        "downloading ${EMBREE_URL}"
        )

    FetchContent_Declare(
        embree
        URL "${EMBREE_URL}"
        # URL_HASH SHA256=<fill in per-platform hash>
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
endif()
FetchContent_MakeAvailable(embree)
