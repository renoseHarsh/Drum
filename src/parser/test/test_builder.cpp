#include <catch2/catch_test_macros.hpp>

import std;

import parser;

namespace drum::parser::test {
  namespace {
    constexpr const char *arg0 = "drum";
    constexpr const char *arg_build = "build";
    constexpr const char *arg_release = "--release";
    constexpr const char *arg_unknown = "--wrong";
  } // namespace

  TEST_CASE("Build command parses successfully") {
    constexpr const char *argv[] = {arg0, arg_build};

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE(result);

    const auto *build_args = std::get_if<builder_cmd::BuildArgs>(&*result);
    REQUIRE(build_args != nullptr);
    REQUIRE_FALSE(build_args->release);
  }

  TEST_CASE("Build command parses release option") {
    constexpr const char *argv[] = {arg0, arg_build, arg_release};

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE(result);

    const auto *build_args = std::get_if<builder_cmd::BuildArgs>(&*result);
    REQUIRE(build_args != nullptr);
    REQUIRE(build_args->release);
  }

  TEST_CASE("Build command rejects unknown arguments") {
    constexpr const char *argv[] = {arg0, arg_build, arg_unknown};

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Build command rejects unknown option") {
    constexpr const char *argv[] = {
        arg0,
        arg_build,
        arg_release,
        arg_unknown,
    };

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE_FALSE(result);
  }
}; // namespace drum::parser::test
