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
  manifest
  MODULES
  src/manifest.cppm
  SOURCES
  src/manifest.cpp
  PRIVATE_LIBRARIES
  tomlplusplus_modules
  TEST_SOURCES
  test/test_manifest.cpp)
