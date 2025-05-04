cmake_minimum_required(VERSION 3.20)
project(game LANGUAGES C CXX)

# ──────────────────────────────────────────────
# 1.  Toolchain flags
# ──────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ──────────────────────────────────────────────
# 2.  Bring in raylib *once*
# ──────────────────────────────────────────────
set(RAYLIB_DIR "C:/raylib/raylib")                 # path to cloned raylib
add_subdirectory("${RAYLIB_DIR}"
                 "${CMAKE_BINARY_DIR}/raylib-build")

#   ↳ after this you get an imported target called `raylib`

# ──────────────────────────────────────────────
# 3.  Game sources
# ──────────────────────────────────────────────
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
     src/*.cpp
     include/*.hpp)

add_executable(game ${GAME_SRC})
target_include_directories(game PRIVATE include)
target_link_libraries(game PRIVATE raylib)

# ──────────────────────────────────────────────
# 4.  Runtime assets next to the .exe
# ──────────────────────────────────────────────
file(COPY rsc DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
