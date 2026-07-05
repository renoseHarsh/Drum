add_library(filesystem)

target_sources(
  filesystem
  PUBLIC FILE_SET CXX_MODULES FILES src/filesystem.cppm
  PRIVATE src/filesystem.cpp)

target_link_libraries(
  filesystem
  PUBLIC file_tree
  PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
