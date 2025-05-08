cmake_minimum_required (VERSION 3.10...3.90)

project (Aspace LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD          17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include (FetchContent)

message(STATUS "Attempting to fetch Raylib 5.5...")

set(FETCHCONTENT_TRY_FIND_PACKAGE_MODE NEVER)
set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    TIMEOUT 120  # Longer timeout (2 minutes)
)

# Rest of your FetchContent setup
set(RAYLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_TESTS    OFF CACHE BOOL "" FORCE)
set(RAYLIB_BUILD_TOOLS    OFF CACHE BOOL "" FORCE)

# Try the download
FetchContent_MakeAvailable(raylib)
message(STATUS "Raylib 5.5 fetch and configuration completed!")

file (GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
      src/*.cpp)

add_executable (Aspace ${GAME_SRC})

target_include_directories (Aspace PRIVATE include)

if (MSVC)
    target_compile_options (Aspace PRIVATE /W4 /permissive-)
else()
    target_compile_options (Aspace PRIVATE -Wall -Wextra -pedantic)
endif()

get_target_property(_RL_INC raylib INTERFACE_INCLUDE_DIRECTORIES)
target_include_directories(Aspace SYSTEM PRIVATE ${_RL_INC})

target_link_libraries (Aspace PRIVATE raylib)

if (WIN32 AND NOT BUILD_SHARED_LIBS)
    target_link_libraries (Aspace PRIVATE
        winmm gdi32 opengl32 shell32)
endif()

add_custom_command (TARGET Aspace POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/rsc"
            "$<TARGET_FILE_DIR:Aspace>/rsc"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/lib"
            "$<TARGET_FILE_DIR:Aspace>"
    COMMENT "Copying game assets + DLLs to the output directory"
)

message (STATUS "------------------------------------------------------------")
message (STATUS "Configured Aspace with raylib 5.5")
message (STATUS "Sources  : ${PROJECT_SOURCE_DIR}")
message (STATUS "Build dir: ${CMAKE_BINARY_DIR}")
message (STATUS "Output   : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Aspace.exe")
message (STATUS "------------------------------------------------------------")
