add_library(parser)

target_sources(parser PRIVATE src/parser.cpp)
target_include_directories(
  parser
  PRIVATE src/
  PUBLIC include/)

target_link_libraries(
  parser
  PUBLIC commands
  PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
