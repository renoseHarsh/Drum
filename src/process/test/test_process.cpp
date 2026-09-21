#include <catch2/catch_test_macros.hpp>

import std;

import process;

namespace drum::process::test {

  TEST_CASE("Invalid command") {
    const auto result =
        run_process("nonexistent_command_123", std::span<std::string>{});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() ==
            std::error_code{ENOENT, std::generic_category()}.message());
  }

  TEST_CASE("Successful command") {
    const std::vector<std::string> args{"1", "=", "1"};
    const auto result = run_process("test", args);
    REQUIRE(result);
    REQUIRE_FALSE(*result);
  }

  TEST_CASE("Failed command") {
    const std::vector<std::string> args{"1", "=", "2"};
    const auto result = run_process("test", args);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().empty());
  }

  TEST_CASE("Capture stdout") {
    const std::vector<std::string> args{"hello", "capture"};
    const auto result = run_process("echo", args, Stdout::capture);
    REQUIRE(result);
    REQUIRE(*result == "hello capture\n");
  }

  TEST_CASE("Failed command with capture") {
    const std::vector<std::string> args{"1", "=", "2"};
    const auto result = run_process("test", args, Stdout::capture);
    REQUIRE_FALSE(result);
    REQUIRE(result.error().empty());
  }
} // namespace drum::process::test
