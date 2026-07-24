include(CMakeParseArguments)

function(drum_add_command)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs MODULES SOURCES PUBLIC_LIBRARIES PRIVATE_LIBRARIES
                     TEST_MODULES TEST_SOURCES)

  cmake_parse_arguments(DRUM "${options}" "${oneValueArgs}" "${multiValueArgs}"
                        ${ARGN})

  if(NOT DRUM_NAME)
    message(FATAL_ERROR "drum_add_command: NAME is required")
  endif()

  #
  # Production library
  #
  add_library(${DRUM_NAME})

  target_sources(
    ${DRUM_NAME}
    PUBLIC FILE_SET CXX_MODULES FILES ${DRUM_MODULES}
    PRIVATE ${DRUM_SOURCES})

  target_link_libraries(
    ${DRUM_NAME}
    PUBLIC ${DRUM_PUBLIC_LIBRARIES}
    PRIVATE ${DRUM_PRIVATE_LIBRARIES} project_warning)

  #
  # Tests
  #
  if(DRUM_TEST_ENABLE AND (DRUM_TEST_MODULES OR DRUM_TEST_SOURCES))
    include(Catch)
    set(test_target "${DRUM_NAME}_tests")

    add_executable(${test_target})

    target_sources(
      ${test_target}
      PUBLIC FILE_SET CXX_MODULES FILES ${DRUM_MODULES} ${DRUM_TEST_MODULES}
      PRIVATE ${DRUM_SOURCES} ${DRUM_TEST_SOURCES})

    target_link_libraries(
      ${test_target}
      PUBLIC ${DRUM_PUBLIC_LIBRARIES}
      PRIVATE ${DRUM_PRIVATE_LIBRARIES} Catch2::Catch2WithMain)

    catch_discover_tests(${test_target})
  endif()
endfunction()
