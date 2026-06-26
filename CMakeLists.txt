cmake_minimum_required(VERSION 4.3)
project(Drum)

add_executable(drum)
target_sources(drum PRIVATE src/main.cpp)
target_include_directories(drum PRIVATE src/)

if(CMAKE_EXPORT_COMPILE_COMMANDS)
  add_custom_command(
    TARGET drum
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E create_symlink
      ${CMAKE_BINARY_DIR}/compile_commands.json
      ${CMAKE_SOURCE_DIR}/compile_commands.json)
endif()
