#include <catch2/catch_test_macros.hpp>

import std;

import parser;

namespace drum::parser::test {
  namespace {
    constexpr const char *arg0 = "drum";
    constexpr const char *arg_new = "new";

    void check_new_args(const std::expected<Command, std::string> &result,
                        new_cmd::NewArgs::PackageType type,
                        std::string_view name) {
      REQUIRE(result.has_value());

      const auto *new_args = std::get_if<new_cmd::NewArgs>(&result.value());

      REQUIRE(new_args != nullptr);
      REQUIRE(new_args->pkg_type == type);
      REQUIRE(new_args->pkg_name == name);
    }
  } // namespace

  TEST_CASE("New command missing package name") {
    constexpr const char *argv[] = {arg0, arg_new};

    const auto result = parser::parse_arguments(2, argv);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "new command missing package name");
  }

  TEST_CASE("New command missing package name with options") {
    constexpr const char *argv[] = {arg0, arg_new, "--lib"};

    const auto result = parser::parse_arguments(3, argv);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "new command missing package name");
  }

  TEST_CASE("New command rejects unknown option") {
    constexpr const char *argv[] = {arg0, arg_new, "--invalid"};

    const auto result = parser::parse_arguments(3, argv);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Unknown option for new: --invalid");
  }

  TEST_CASE("New command rejects multiple package names") {
    constexpr const char *argv[] = {arg0, arg_new, "pkg", "another"};

    const auto result = drum::parser::parse_arguments(4, argv);

    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Unknown arg: another");
  }

  TEST_CASE("New command defaults to executable package type") {
    constexpr const char *argv[] = {arg0, arg_new, "pkg"};

    const auto result = drum::parser::parse_arguments(3, argv);
    check_new_args(result, new_cmd::NewArgs::PackageType::executable, "pkg");
  }

  TEST_CASE("New command with --lib parses as library package type") {
    constexpr const char *argv[] = {arg0, arg_new, "pkg", "--lib"};

    const auto result = drum::parser::parse_arguments(4, argv);
    check_new_args(result, new_cmd::NewArgs::PackageType::library, "pkg");
  }

} // namespace drum::parser::test
