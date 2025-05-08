cmake_minimum_required (VERSION 3.10...3.90)

project (Aspace LANGUAGES C CXX)
set (CMAKE_CXX_STANDARD          17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

include (FetchContent)

set(RAYLIB_BINARY_URL "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip")
set(RAYLIB_MIRROR_URL "https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip")

message(STATUS "Attempting to download pre-built Raylib 5.5 mingw binaries...")

option(LOCAL_RAYLIB "Use local Raylib binaries" OFF)
set(RAYLIB_LOCAL_PATH "${CMAKE_SOURCE_DIR}/lib/raylib-5.5_win64_msvc16.zip" CACHE STRING "Path to local Raylib binaries")

if(LOCAL_RAYLIB AND EXISTS "${RAYLIB_LOCAL_PATH}")
    message(STATUS "Using local Raylib binaries from: ${RAYLIB_LOCAL_PATH}")
    FetchContent_Declare(
        raylib_binaries
        URL "${RAYLIB_LOCAL_PATH}"
    )
else()
    message(STATUS "Downloading Raylib from: ${RAYLIB_BINARY_URL}")
    FetchContent_Declare(
        raylib_binaries
        URL ${RAYLIB_BINARY_URL}
        TLS_VERIFY ON
    )
endif()

# Make the content available
FetchContent_MakeAvailable(raylib_binaries)

# Get paths to the downloaded content
set(RAYLIB_SOURCE_DIR "${raylib_binaries_SOURCE_DIR}")
set(RAYLIB_INCLUDE_DIR "${RAYLIB_SOURCE_DIR}/include")
set(RAYLIB_LIBRARY_DIR "${RAYLIB_SOURCE_DIR}/lib")

# Create an imported target for the library
add_library(raylib SHARED IMPORTED)
set_target_properties(raylib PROPERTIES
    IMPORTED_IMPLIB         "${RAYLIB_LIBRARY_DIR}/libraylibdll.a"
    IMPORTED_LOCATION       "${RAYLIB_LIBRARY_DIR}/raylib.dll"
    INTERFACE_INCLUDE_DIRECTORIES "${RAYLIB_INCLUDE_DIR}"
)

message(STATUS "Raylib binaries located at: ${RAYLIB_SOURCE_DIR}")
message(STATUS "Raylib include directory: ${RAYLIB_INCLUDE_DIR}")
message(STATUS "Raylib library directory: ${RAYLIB_LIBRARY_DIR}")

# Collect source files
file(GLOB_RECURSE GAME_SRC CONFIGURE_DEPENDS
     src/*.cpp)

add_executable(Aspace ${GAME_SRC})

target_include_directories(Aspace PRIVATE include ${RAYLIB_INCLUDE_DIR})

if(MSVC)
    target_compile_options(Aspace PRIVATE /W4 /permissive-)
else()
    target_compile_options(Aspace PRIVATE -Wall -Wextra -pedantic)
endif()

target_link_libraries(Aspace PRIVATE raylib)

if(WIN32)
    target_link_libraries(Aspace PRIVATE
        winmm gdi32 opengl32 shell32)
endif()

# Copy DLLs and resources
add_custom_command (TARGET Aspace POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_SOURCE_DIR}/rsc"
            "$<TARGET_FILE_DIR:Aspace>/rsc"
    # Copy raylib DLL if it exists
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${RAYLIB_LIBRARY_DIR}/raylib.dll"
            "$<TARGET_FILE_DIR:Aspace>/raylib.dll"
    COMMENT "Copying game assets + DLLs to the output directory"
)

message (STATUS "------------------------------------------------------------")
message (STATUS "Configured Aspace with pre-built raylib 5.5")
message (STATUS "Sources  : ${PROJECT_SOURCE_DIR}")
message (STATUS "Build dir: ${CMAKE_BINARY_DIR}")
message (STATUS "Output   : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Aspace.exe")
message (STATUS "------------------------------------------------------------")