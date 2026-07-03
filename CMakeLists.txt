cmake_minimum_required(VERSION 4.3)

project(Drum LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

if(DRUM_TEST_ENABLE OR DRUM_BENCHMARK_ENABLE)
  find_package(Catch2 CONFIG REQUIRED)
  include(Catch)
endif()

add_library(project_warning INTERFACE)

target_compile_options(
  project_warning
  INTERFACE -Wall
            -Wextra
            -Wpedantic
            -Wconversion
            -Wsign-conversion
            -Wnon-virtual-dtor
            -Wold-style-cast)

add_subdirectory(drum)
add_subdirectory(parser)
add_subdirectory(executor)
add_subdirectory(commands)
add_subdirectory(file_tree)
add_subdirectory(blueprint)
add_subdirectory(filesystem)
