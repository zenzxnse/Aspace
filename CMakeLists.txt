cmake_minimum_required(VERSION 3.20)
project(space_something LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

include(FetchContent)
#  ── pull the *current* raylib instead of 5.0.0 ───────────────
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        master           # <- or a fixed commit / tag ≥ 5.1
)
FetchContent_MakeAvailable(raylib)   # provides target 'raylib'

file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
     src/*.cpp
     include/*.hpp)
add_executable(game ${GAME_SRC})
target_include_directories(game PRIVATE include)

# link system libs explicitly on Windows when using static raylib
if (WIN32 AND NOT raylib_SHARED)
    target_link_libraries(game PRIVATE raylib gdi32 winmm opengl32)
else()
    target_link_libraries(game PRIVATE raylib)
endif()
target_compile_definitions(game PRIVATE PLATFORM_DESKTOP)

# copy assets after build (so “bin/game.exe” finds rsc folder)
add_custom_command(TARGET game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/rsc"
            "$<TARGET_FILE_DIR:game>/rsc")

set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
