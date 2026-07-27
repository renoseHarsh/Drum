module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_validate;

import std;

import :validate;
import test_util;

namespace drum::builder_cmd::validate::test {

  namespace {
    void require_error(std::string_view expected) {
      const auto result = validate();
      REQUIRE_FALSE(result);
      REQUIRE(result.error() == expected);
    }

  } // namespace

  TEST_CASE("Missing drum.toml") { require_error("Missing drum.toml"); }

  TEST_CASE("Invalid TOML syntax") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml", "[[unclosed");
    const auto result = validate();
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Missing name in manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml", "version = \"0.1.0\"\ntype = \"exec\"");
    require_error("Missing name");
  }

  TEST_CASE("Missing version in manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml", "name = \"test_pkg\"\ntype = \"exec\"");
    require_error("Missing version");
  }

  TEST_CASE("Missing type in manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml",
                          "name = \"test_pkg\"\nversion = \"0.1.0\"");
    require_error("Missing type");
  }

  TEST_CASE("Non-string type value") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml", "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = 42");
    require_error("Invalid type");
  }

  TEST_CASE("Unrecognized type value") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"foo\"");
    require_error("Invalid type");
  }

  TEST_CASE("Exec package missing src directory") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    require_error("Invalid executable package layout");
  }

  TEST_CASE("Exec package missing main.cpp") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    std::filesystem::create_directory("src");
    require_error("Invalid executable package layout");
  }

  TEST_CASE("Valid executable package") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    std::filesystem::create_directory("src");
    test_util::write_file("src/main.cpp", "int main() {}");
    const auto result = validate();
    REQUIRE(result);
  }

  TEST_CASE("Lib type not implemented") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"lib\"");
    require_error("Not implemented lib");
  }

} // namespace drum::builder_cmd::validate::test
