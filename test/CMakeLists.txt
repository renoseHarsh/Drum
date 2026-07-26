add_library(test_utils)

target_sources(
  test_utils
  PUBLIC FILE_SET CXX_MODULES FILES test_util.cppm)

target_link_libraries(test_utils PUBLIC Catch2::Catch2WithMain)
