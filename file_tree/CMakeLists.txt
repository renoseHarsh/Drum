add_library(file_tree)

target_sources(
  file_tree
  PUBLIC FILE_SET CXX_MODULES FILES src/file_tree.cppm
  PRIVATE src/file_tree.cpp)

target_link_libraries(file_tree PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
