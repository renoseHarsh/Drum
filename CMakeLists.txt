cmake_minimum_required(VERSION 4.3)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPERIMENTAL_CXX_IMPORT_STD "f35a9ac6-8463-4d38-8eec-5d6008153e7d")
set(CMAKE_CXX_MODULE_STD ON)

project(Drum LANGUAGES CXX)

set(CPM_DOWNLOAD_VERSION 0.43.1)
set(CPM_DOWNLOAD_LOCATION
    "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")

if(NOT (EXISTS ${CPM_DOWNLOAD_LOCATION}))
  message(STATUS "Downloading CPM.cmake")
  file(
    DOWNLOAD
    https://github.com/TheLartians/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
    ${CPM_DOWNLOAD_LOCATION})
endif()

set(CPM_SOURCE_CACHE "$ENV{HOME}/.cache/CPM")
include(${CPM_DOWNLOAD_LOCATION})

enable_testing()

if(DRUM_TEST_ENABLE OR DRUM_BENCHMARK_ENABLE)
  cpmaddpackage(NAME Catch2 GITHUB_REPOSITORY catchorg/Catch2 VERSION 3.15.3)
  list(APPEND CMAKE_MODULE_PATH "${Catch2_SOURCE_DIR}/extras")
  include(Catch)
  add_subdirectory(test)
endif()

include(${CMAKE_SOURCE_DIR}/cmake/DrumCommand.cmake)

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
add_subdirectory(new)
add_subdirectory(builder)
add_subdirectory(run)
add_subdirectory(manifest)
