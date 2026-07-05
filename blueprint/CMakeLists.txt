add_library(blueprint)

target_sources(
  blueprint
  PUBLIC FILE_SET CXX_MODULES FILES src/blueprint.cppm
  PRIVATE src/blueprint.cpp)

target_link_libraries(
  blueprint
  PUBLIC file_tree
  PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
