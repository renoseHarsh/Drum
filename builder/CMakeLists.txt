cpmaddpackage(
  NAME
  tomlplusplus
  GITHUB_REPOSITORY
  marzer/tomlplusplus
  GIT_TAG
  master
  OPTIONS
  "TOMLPLUSPLUS_BUILD_MODULES ON"
  "TOML_EXCEPTIONS OFF")

drum_add_command(
  NAME
  builder_cmd
  MODULES
  src/builder_cmd.cppm
  SOURCES
  src/builder_cmd.cpp
  PRIVATE_LIBRARIES
  tomlplusplus_modules)
