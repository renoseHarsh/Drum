module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_process;

import std;

import :process;

namespace drum::builder_cmd::process::test {

  TEST_CASE("Invalid command") {
    const std::vector<std::string> args{"nonexistent_command_123"};
    const auto result = run_process(args);
    REQUIRE_FALSE(result);
    REQUIRE(result.error() ==
            std::error_code{ENOENT, std::generic_category()}.message());
  }

  TEST_CASE("Successful command") {
    const std::vector<std::string> args{"test", "1", "=", "1"};
    const auto result = run_process(args);
    REQUIRE(result);
  }

  TEST_CASE("Failed command") {
    const std::vector<std::string> args{"test", "1", "=", "2"};
    const auto result = run_process(args);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().empty());
  }

} // namespace drum::builder_cmd::process::test
