cmake_minimum_required(VERSION 4.3)
project(Drum)
enable_testing()
if(DRUM_TEST_ENABLE)
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
