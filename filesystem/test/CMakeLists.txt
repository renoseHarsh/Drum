add_executable(test_filesystem)

target_sources(test_filesystem PRIVATE test_filesystem.cpp)

target_link_libraries(test_filesystem PRIVATE filesystem Catch2::Catch2WithMain)

catch_discover_tests(test_filesystem)
