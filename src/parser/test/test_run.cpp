#include <catch2/catch_test_macros.hpp>

import std;

import parser;

namespace drum::parser::test {

  namespace {

    constexpr const char *arg0 = "drum";
    constexpr const char *arg_run = "run";
    constexpr const char *arg_release = "--release";
    constexpr const char *arg_profile = "--profile";
    constexpr const char *arg_unknown = "--wrong";

  } // namespace

  TEST_CASE("Run command parses successfully") {
    constexpr const char *argv[] = {arg0, arg_run};

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE(result);

    const auto *build_args = std::get_if<run_cmd::RunArgs>(&*result);
    REQUIRE(build_args != nullptr);
    REQUIRE(build_args->profile == "debug");
  }

  TEST_CASE("Run command parses release option") {
    constexpr const char *argv[] = {arg0, arg_run, arg_release};

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE(result);

    const auto *build_args = std::get_if<run_cmd::RunArgs>(&*result);
    REQUIRE(build_args != nullptr);
    REQUIRE(build_args->profile == "release");
  }

  TEST_CASE("Run command rejects unknown arguments") {
    constexpr const char *argv[] = {arg0, arg_run, arg_unknown};

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Run command rejects unknown option") {
    constexpr const char *argv[] = {
        arg0,
        arg_run,
        arg_release,
        arg_unknown,
    };

    auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Run command rejects --profile without a value") {
    constexpr const char *argv[] = {arg0, arg_run, arg_profile};

    const auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Missing profile name");
  }

  TEST_CASE("Run command parses --profile with a custom name") {
    constexpr const char *argv[] = {arg0, arg_run, arg_profile, "custom"};

    const auto result = drum::parser::parse_arguments(std::size(argv), argv);
    REQUIRE(result);

    const auto *build_args = std::get_if<run_cmd::RunArgs>(&*result);
    REQUIRE(build_args != nullptr);
    REQUIRE(build_args->profile == "custom");
  }

}; // namespace drum::parser::test
