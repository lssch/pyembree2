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

    FetchContent_MakeAvailable(embree)
else()
    set(EMBREE_URL
        "https://github.com/RenderKit/embree/releases/download/v${EMBREE_VERSION}/${EMBREE_ARCHIVE}"
    )

    message(STATUS
        "Prebuilt Embree available for "
        "${CMAKE_SYSTEM_NAME}/${CMAKE_SYSTEM_PROCESSOR}; "
        "downloading ${EMBREE_URL}"
    )

    set(EMBREE_ROOT
        "${CMAKE_BINARY_DIR}/_embree"
    )

    file(REMOVE_RECURSE "${EMBREE_ROOT}")
    file(MAKE_DIRECTORY "${EMBREE_ROOT}")

    set(EMBREE_ARCHIVE_PATH
        "${CMAKE_BINARY_DIR}/${EMBREE_ARCHIVE}"
    )

    file(DOWNLOAD
        "${EMBREE_URL}"
        "${EMBREE_ARCHIVE_PATH}"
        SHOW_PROGRESS
        TLS_VERIFY ON
    )

    file(ARCHIVE_EXTRACT
        INPUT "${EMBREE_ARCHIVE_PATH}"
        DESTINATION "${EMBREE_ROOT}"
    )

    file(GLOB EMBREE_DIRS
        "${EMBREE_ROOT}/*"
    )
    if(NOT EXISTS "${EMBREE_ROOT}/include")
        list(GET EMBREE_DIRS 0 EMBREE_ROOT)
    endif()

    if(WIN32)
        add_library(embree UNKNOWN IMPORTED GLOBAL)
        set_target_properties(embree PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EMBREE_ROOT}/include"
            IMPORTED_IMPLIB "${EMBREE_ROOT}/lib/embree4.lib"
            IMPORTED_LOCATION "${EMBREE_ROOT}/bin/embree4.dll"
        )
    elseif(APPLE)
        add_library(embree SHARED IMPORTED GLOBAL)
        set_target_properties(embree PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EMBREE_ROOT}/include"
            IMPORTED_LOCATION "${EMBREE_ROOT}/lib/libembree4.dylib"
        )
    else()
        add_library(embree SHARED IMPORTED GLOBAL)
        set_target_properties(embree PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${EMBREE_ROOT}/include"
            IMPORTED_LOCATION "${EMBREE_ROOT}/lib/libembree4.so"
        )
    endif()
endif()
