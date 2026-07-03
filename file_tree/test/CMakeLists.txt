add_executable(test_node)

target_sources(test_node PRIVATE test_node.cpp)

target_link_libraries(test_node PRIVATE file_tree project_warning
                                        Catch2::Catch2WithMain)

catch_discover_tests(test_node)
