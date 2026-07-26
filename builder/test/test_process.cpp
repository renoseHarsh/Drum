module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_process;

import std;

import :process;

namespace drum::builder_cmd::process::test {

  TEST_CASE("Invalid command") {
    std::vector<std::string> args = {"nonexistent_command_123"};
    auto result = run_process(args);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            std::error_code{ENOENT, std::generic_category()}.message());
  }

  TEST_CASE("Successful command") {
    std::vector<std::string> args = {"test", "1", "=", "1"};
    auto result = run_process(args);
    REQUIRE(result.has_value());
  }

  TEST_CASE("Failed command") {
    std::vector<std::string> args = {"test", "1", "=", "2"};
    auto result = run_process(args);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error().empty());
  }

} // namespace drum::builder_cmd::process::test
