#include "parser/parser.hpp"
#include <catch2/catch_test_macros.hpp>

constexpr const char *arg0 = "drum";
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

constexpr const char *arg_new = "new";
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

  const auto result = parser::parse_arguments(4, argv);

  REQUIRE_FALSE(result.has_value());
  REQUIRE(result.error() == "Unknown arg: another");
}

auto check_new_args = [](const auto &result, args::NewArgs::PackageType type,
                         std::string_view name) {
  REQUIRE(result.has_value());

  const auto *new_args = std::get_if<args::NewArgs>(&result.value());
  REQUIRE(new_args != nullptr);
  REQUIRE(new_args->pkg_type == type);
  REQUIRE(new_args->pkg_name == name);
};

TEST_CASE("New command creates executable package") {
  constexpr const char *argv[] = {arg0, arg_new, "pkg"};

  const auto result = parser::parse_arguments(3, argv);
  check_new_args(result, args::NewArgs::PackageType::executable, "pkg");
}

TEST_CASE("New command creates library package") {
  constexpr const char *argv[] = {arg0, arg_new, "pkg", "--lib"};

  const auto result = parser::parse_arguments(4, argv);
  check_new_args(result, args::NewArgs::PackageType::library, "pkg");
}
