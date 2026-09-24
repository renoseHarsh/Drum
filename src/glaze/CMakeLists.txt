cpmaddpackage("gh:stephenberry/glaze@8.3.0")

drum_add_command(
  NAME
  glaze
  MODULES
  glaze.cppm
  p1689_types.cppm
  PRIVATE_LIBRARIES
  glaze::glaze)
