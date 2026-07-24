drum_add_command(
  NAME
  parser
  MODULES
  src/parser.cppm
  SOURCES
  src/parser.cpp
  PUBLIC_LIBRARIES
  new_cmd
  TEST_SOURCES
  test/test_parser.cpp
  test/test_new.cpp)

target_link_libraries(parser PUBLIC new_cmd)
