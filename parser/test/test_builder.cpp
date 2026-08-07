#include <catch2/catch_test_macros.hpp>

import std;

import parser;

namespace drum::parser::test {
  namespace {
    constexpr const char *arg0 = "drum";
    constexpr const char *arg_build = "build";
  } // namespace

  TEST_CASE("Build command parses successfully") {
    constexpr const char *argv[] = {arg0, arg_build};

    auto result = drum::parser::parse_arguments(2, argv);

    REQUIRE(result);

    const auto *build_args = std::get_if<builder_cmd::BuildArgs>(&*result);

    REQUIRE(build_args != nullptr);
  }
}; // namespace drum::parser::test
