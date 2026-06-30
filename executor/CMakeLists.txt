add_library(executor)

target_sources(executor PRIVATE src/executor.cpp)
target_include_directories(
  executor
  PRIVATE src/
  PUBLIC include/)

target_link_libraries(
  executor
  PUBLIC commands
  PRIVATE project_warning blueprint filesystem)
