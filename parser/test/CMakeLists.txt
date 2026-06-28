add_executable(test_parser)

target_sources(test_parser PRIVATE test_parser.cpp)
target_include_directories(test_parser PRIVATE ../src)

target_link_libraries(test_parser PRIVATE parser Catch2::Catch2WithMain)

catch_discover_tests(test_parser)
