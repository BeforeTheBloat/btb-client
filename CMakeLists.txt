cmake_minimum_required(VERSION 3.24.0)
project(btb-client VERSION 1.0 LANGUAGES CXX)

include(FetchContent)

set(CMAKE_CXX_STANDARD 23)

if(MSVC)
    add_compile_options(/MT)

    add_compile_options($<$<CONFIG:Release>:/O2>)
    add_compile_options($<$<CONFIG:Release>:/Ot>)
    add_compile_options($<$<CONFIG:Release>:/Ox>)
    add_compile_options($<$<CONFIG:Release>:/Oy>)
    add_compile_options($<$<CONFIG:Release>:/Oi>)
    add_compile_options($<$<CONFIG:Release>:/GL>)
    add_compile_options($<$<CONFIG:Release>:/Gy>)
    add_compile_options($<$<CONFIG:Release>:/Gw>)
    add_compile_options($<$<CONFIG:Release>:/GF>)
    add_compile_options($<$<CONFIG:Release>:/GR->)
    add_compile_options($<$<CONFIG:Release>:/Ob2>)

    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
endif()

# FetchContent declarations
FetchContent_Declare(
    libhat
    GIT_REPOSITORY  https://github.com/BasedInc/libhat.git
    GIT_TAG         f4755aea0987e77c0f6f496c49eb9cd08d5f5a06
)

FetchContent_Declare(
    minhook
    GIT_REPOSITORY  https://github.com/TsudaKageyu/minhook
    GIT_TAG         c1a7c3843bd1a5fe3eb779b64c0d823bca3dc339
)

FetchContent_Declare(
    magic_enum
    GIT_REPOSITORY  https://github.com/Neargye/magic_enum
    GIT_TAG         dae6bbf16c363e9ead4e628a47fdb02956a634f3
)

FetchContent_Declare(
    entt
    GIT_REPOSITORY  https://github.com/skypjack/entt
    GIT_TAG         f931687ff04d435871ac9664bb299f71f2a8fafc
)

FetchContent_Declare(
    magic_vtable
    GIT_REPOSITORY  https://github.com/DisabledMallis/magic_vtable.git
    GIT_TAG         main
)

FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm
    GIT_TAG         1.0.1
)

FetchContent_Declare(
    nes
    GIT_REPOSITORY  https://github.com/DisabledMallis/NuvolaEventSystem.git
    GIT_TAG         v1.1.0
)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY  https://github.com/ocornut/imgui.git
    GIT_TAG         2ee4111f04121297eebc33b9ac69763c8995c67b
)

FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
endif()

FetchContent_MakeAvailable(libhat minhook magic_enum entt magic_vtable glm nes imgui)

# Define the btb-client target
file(GLOB_RECURSE sources "src/dllmain.cpp" "src/pch.cpp" "src/**/*.cpp" "src/**/*.hpp" "src/**/*.h")
add_library(${PROJECT_NAME} SHARED ${sources})

# Include and link directories
target_include_directories(${PROJECT_NAME} PRIVATE ${imgui_SOURCE_DIR})

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    windowscodecs.lib
    urlmon.lib
    dwrite.lib
    dxgi.lib
    d3d11.lib
    wininet.lib
    libhat
    minhook
    magic_enum
    EnTT
    magic_vtable
    glm
    NES
    version
)

# Set compile options
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -fno-rtti)
elseif (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /GR-)
endif()
