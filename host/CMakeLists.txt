cmake_minimum_required(VERSION 3.20)
project(TesseraHost LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)

# Force 32-bit build
if(MSVC)
    # Verify we're targeting x86
    if(NOT CMAKE_SIZEOF_VOID_P EQUAL 4)
        message(FATAL_ERROR "TesseraHost must be built as 32-bit (x86). Use -A Win32.")
    endif()
endif()

# External dependencies (git submodules)
# Note: adding new submodules requires `git submodule update --init --recursive`
add_subdirectory(external/pugixml)

set(YYJSON_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YYJSON_BUILD_MISC OFF CACHE BOOL "" FORCE)
add_subdirectory(external/yyjson)

# Host executable
file(GLOB_RECURSE HOST_SOURCES src/*.cpp)
add_executable(TesseraHost ${HOST_SOURCES})

target_include_directories(TesseraHost PRIVATE src src/bridge relay/include)
target_link_libraries(TesseraHost PRIVATE pugixml yyjson ws2_32)

# Windows-specific
if(WIN32)
    target_compile_definitions(TesseraHost PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()
