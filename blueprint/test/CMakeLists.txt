add_executable(test_blueprint)

target_sources(test_blueprint PRIVATE test_blueprint.cpp)

target_link_libraries(test_blueprint PRIVATE blueprint project_warning
                                             Catch2::Catch2WithMain)

catch_discover_tests(test_blueprint)
