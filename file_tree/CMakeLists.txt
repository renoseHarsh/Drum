add_library(file_tree)

target_sources(file_tree PRIVATE src/file_tree.cpp)
target_include_directories(
  file_tree
  PRIVATE src/
  PUBLIC include/)

target_link_libraries(file_tree PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
