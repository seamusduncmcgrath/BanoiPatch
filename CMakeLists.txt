cmake_minimum_required(VERSION 4.2)
project(BanoiPatch)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")

add_library(MinHook STATIC
        libs/MinHook/src/buffer.c
        libs/MinHook/src/hook.c
        libs/MinHook/src/trampoline.c
        libs/MinHook/src/hde32.c
)
target_include_directories(MinHook PUBLIC libs/MinHook/include)

include(FetchContent)
FetchContent_Declare(
        SDL3
        GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
        GIT_TAG        release-3.2.10
        GIT_SHALLOW    TRUE
)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
set(SDL_TEST   OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(SDL3)

add_library(xinput1_3 SHARED
        src/dllmain.cpp
        xinput1_3.def
        src/Utils.cpp
        src/D3DHooks.cpp
        src/Config.cpp
)

target_include_directories(xinput1_3 PRIVATE
        src
        ${CMAKE_SOURCE_DIR}
)

target_link_libraries(xinput1_3 PRIVATE
        MinHook
        SDL3::SDL3-static
        d3d9
        dxguid
        dinput8
)

set_target_properties(xinput1_3 PROPERTIES
        OUTPUT_NAME "xinput1_3"
        SUFFIX ".dll"
)

