cmake_minimum_required(VERSION 3.20)
project(space_something LANGUAGES C CXX)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  raylib
  URL      https://github.com/raysan5/raylib/archive/refs/tags/5.0.tar.gz  # ← fixed
  # or: GIT_REPOSITORY https://github.com/raysan5/raylib.git
  #     GIT_TAG        5.0
)
FetchContent_MakeAvailable(raylib)        # produces target “raylib”

file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS src/*.cpp include/*.hpp)
add_executable(game ${GAME_SRC})
target_include_directories(game PRIVATE include)
target_compile_definitions(game PRIVATE PLATFORM_DESKTOP)

# link system libs once on Windows if you use the static raylib build
if (WIN32 AND NOT raylib_SHARED)
  target_link_libraries(game PRIVATE raylib opengl32 gdi32 winmm)          # :contentReference[oaicite:3]{index=3}
else()
  target_link_libraries(game PRIVATE raylib)
endif()

# drop the exe in ./bin and copy assets right next to it
set_target_properties(game PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_custom_command(TARGET game POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy_directory
          "${CMAKE_SOURCE_DIR}/rsc"
          "$<TARGET_FILE_DIR:game>/rsc")
