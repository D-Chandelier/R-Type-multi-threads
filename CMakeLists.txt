cmake_minimum_required(VERSION 3.21)
project(RType LANGUAGES CXX)

# ───────────────────────────────────────────── 
# Désactive tous les Deprecation Warnings 
# ───────────────────────────────────────────── 
if(NOT DEFINED CMAKE_WARN_DEPRECATED)
    set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

cmake_policy(SET CMP0048 NEW)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

# ─────────────────────────────────────────────
# CPM.cmake
# ─────────────────────────────────────────────
if(NOT EXISTS "${CMAKE_BINARY_DIR}/cmake/CPM.cmake")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/latest/download/CPM.cmake
        "${CMAKE_BINARY_DIR}/cmake/CPM.cmake"
        EXPECTED_HASH SHA256=2020b4fc42dba44817983e06342e682ecfc3d2f484a581f11cc5731fbe4dce8a
    )
endif()
include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)

# ─────────────────────────────────────────────
# SFML 2.6.2
# ─────────────────────────────────────────────

CPMAddPackage(
    NAME SFML
    GITHUB_REPOSITORY SFML/SFML
    GIT_TAG 3.0.2

    OPTIONS
        "SFML_BUILD_TESTS OFF"
        "SFML_BUILD_EXAMPLES OFF"
        "SFML_USE_STATIC_STD_LIBS OFF"
        "SFML_STATIC_LIBRARIES OFF"
)
# Path des DLL SFML
set(SFML_BIN_DIR "${SFML_BINARY_DIR}/lib")

# ─────────────────────────────────────────────
# ENet (1.3.17)
# ─────────────────────────────────────────────

CPMAddPackage(
    NAME ENet
    GITHUB_REPOSITORY lsalzman/enet
    GIT_TAG v1.3.17
    OPTIONS
        "ENET_ENABLE_SHARED OFF"
        "ENET_BUILD_TESTS OFF"
)
if(MSVC AND ENet_ADDED)
    target_compile_definitions(enet PRIVATE 
        _CRT_SECURE_NO_WARNINGS
        _WINSOCK_DEPRECATED_NO_WARNINGS
    )
    target_compile_options(enet PRIVATE 
        /wd4996  # gethostbyname, etc.
        /wd4267  # conversions
        /wd4244
        /wd4365
        /wd4334
        /wd5219
        /wd5287  # ENUM bitmask (celui qui t’embête)
    )
endif()

# ─────────────────────────────────────────────
# Exécutable
# ─────────────────────────────────────────────
file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(RType ${SOURCES})

target_link_libraries(RType
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-network
        sfml-audio
        sfml-system
        enet
)
target_include_directories(RType PRIVATE ${ENET_INCLUDE_DIR}/Include)
target_include_directories(RType PRIVATE ${ENet_SOURCE_DIR}/include)
if (WIN32)
    target_link_libraries(RType PRIVATE ws2_32 winmm)
endif()


# ─────────────────────────────────────────────
# Copie du dossier assets
# ─────────────────────────────────────────────

set(ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
set(BUILD_ASSETS_DIR $<TARGET_FILE_DIR:RType>/assets)


add_custom_command(TARGET RType POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${ASSETS_DIR} ${BUILD_ASSETS_DIR})
