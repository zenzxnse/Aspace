cmake_minimum_required (VERSION 3.20)

# ──────────────────────────────── project ────────────────────────────────
project (Aspace LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD          17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
# put the final .exe into   build/bin/…
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# ────────────────────────────── raylib via FC ────────────────────────────
include (FetchContent)

# 1) Declare what we want to fetch
FetchContent_Declare (
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5         # official 5.5 tag
)

# 2) Cut all the optional extras – we only need the core library
set (RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set (RAYLIB_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set (RAYLIB_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)

# 3) Pull + add_subdirectory() automatically
FetchContent_MakeAvailable (raylib)

# ─────────────────────────────── sources ────────────────────────────────
file (GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
      src/*.cpp)

add_executable (Aspace ${GAME_SRC})

# ─────────────────────────── include / compile flags ─────────────────────
target_include_directories (Aspace PRIVATE include)

if (MSVC)
    target_compile_options (Aspace PRIVATE /W4 /permissive-)
else()
    target_compile_options (Aspace PRIVATE -Wall -Wextra -pedantic)
endif()

# Hide Raylib’s headers as “system” includes
get_target_property(_RL_INC raylib INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(Aspace SYSTEM PRIVATE ${_RL_INC})

# ─────────────────────────── link libraries ──────────────────────────────
target_link_libraries (Aspace PRIVATE raylib)


# Extra Win32 system libs required by raylib when built statically
if (WIN32 AND NOT BUILD_SHARED_LIBS)
    target_link_libraries (Aspace PRIVATE
        winmm gdi32 opengl32 shell32)
endif()

# ─────────────────────────────── assets ──────────────────────────────────
# Copy game assets and extra DLLs next to the executable after each build
add_custom_command (TARGET Aspace POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/rsc"
            "$<TARGET_FILE_DIR:Aspace>/rsc"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/lib"
            "$<TARGET_FILE_DIR:Aspace>"
    COMMENT "Copying game assets + DLLs to the output directory"
)

# ─────────────────────────────── summary ─────────────────────────────────
message (STATUS "------------------------------------------------------------")
message (STATUS "Configured Aspace with raylib 5.5")
message (STATUS "Sources  : ${PROJECT_SOURCE_DIR}")
message (STATUS "Build dir: ${CMAKE_BINARY_DIR}")
message (STATUS "Output   : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Aspace.exe")
message (STATUS "------------------------------------------------------------")

# ───────────────────────── OPTIONAL: pre‑built raylib ────────────────────
# If you’d rather link against the small pre‑built Win‑x64 MinGW package
# (≈ 1.6 MB) instead of compiling raylib yourself, comment‑out the whole
# FetchContent block above and replace it with something like:
#
# find_library(RAYLIB_LIB NAMES raylib PATHS "${CMAKE_SOURCE_DIR}/external/raylib/lib")
# target_link_libraries(Aspace PRIVATE ${RAYLIB_LIB})
# target_include_directories(Aspace PRIVATE "${CMAKE_SOURCE_DIR}/external/raylib/include")
#
# …where `external/raylib` is the unzipped “raylib‑5.5_win64_mingw‑w64.zip”.
