import parser;

import std;

#include <catch2/catch_test_macros.hpp>

namespace drum::parser::test {
  namespace {
    constexpr const char *arg0 = "drum";
  } // namespace

  TEST_CASE("Missing Command") {
    constexpr const char *argv[] = {arg0};

    const auto result = parser::parse_arguments(1, argv);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Provide a command");
  }

  TEST_CASE("Unknown command") {
    constexpr const char *argv[] = {arg0, "invalid"};

    const auto result = parser::parse_arguments(2, argv);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Unknown command: invalid");
  }

} // namespace drum::parser::test
