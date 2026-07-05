cmake_minimum_required(VERSION 4.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "451f2fe2-a8a2-47c3-bc32-94786d8fc91b")
set(CMAKE_CXX_MODULE_STD ON)

project(Drum LANGUAGES CXX)

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
