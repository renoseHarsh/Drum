add_compile_definitions(TOML_EXCEPTIONS=0)
cpmaddpackage(
  NAME
  tomlplusplus
  GITHUB_REPOSITORY
  marzer/tomlplusplus
  GIT_TAG
  master
  OPTIONS
  "TOMLPLUSPLUS_BUILD_MODULES ON")

drum_add_command(
  NAME
  builder_cmd
  MODULES
  src/builder_cmd.cppm
  src/validate.cppm
  src/discover.cppm
  src/compile.cppm
  src/link.cppm
  src/process.cppm
  SOURCES
  src/builder_cmd.cpp
  PRIVATE_LIBRARIES
  tomlplusplus_modules
  TEST_MODULES
  test/test_validate.cpp)
