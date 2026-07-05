add_library(executor)

target_sources(
  executor
  PUBLIC FILE_SET CXX_MODULES FILES src/executor.cppm
  PRIVATE src/executor.cpp)

target_link_libraries(
  executor
  PUBLIC commands
  PRIVATE blueprint filesystem project_warning)
