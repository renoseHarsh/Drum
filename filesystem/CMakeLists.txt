add_library(filesystem)

target_sources(filesystem PRIVATE src/filesystem.cpp)
target_include_directories(
  filesystem
  PRIVATE src/
  PUBLIC include/)

target_link_libraries(
  filesystem
  PUBLIC file_tree
  PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
