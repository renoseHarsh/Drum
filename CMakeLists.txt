cmake_minimum_required(VERSION 4.3)
project(Drum)
enable_testing()

add_executable(drum)
target_sources(drum PRIVATE src/main.cpp)
target_include_directories(drum PRIVATE src/)
target_link_libraries(drum PRIVATE parser command)

add_library(parser)
target_sources(parser PRIVATE src/parser/parser.cpp)
target_include_directories(parser PRIVATE src/)

add_library(command)
target_sources(command PRIVATE src/command/command.cpp)
target_include_directories(command PRIVATE src/)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  add_custom_command(
    TARGET drum
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
