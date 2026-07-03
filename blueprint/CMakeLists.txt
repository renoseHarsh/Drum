add_library(blueprint)

target_sources(blueprint PRIVATE src/blueprint.cpp)
target_include_directories(
  blueprint
  PRIVATE src/
  PUBLIC include/)

target_link_libraries(
  blueprint
  PUBLIC file_tree
  PRIVATE project_warning)

if(DRUM_TEST_ENABLE)
  add_subdirectory(test)
endif()
