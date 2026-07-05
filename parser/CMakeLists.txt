add_library(parser)

target_sources(
  parser
  PUBLIC FILE_SET CXX_MODULES FILES src/parser.cppm
  PRIVATE src/parser.cpp)

target_link_libraries(
  parser
  PUBLIC commands
  PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
