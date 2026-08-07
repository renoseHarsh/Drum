#include <catch2/catch_test_macros.hpp>

import std;

import manifest;
import test_util;

namespace drum::manifest::test {
  namespace {
    void require_error(std::string_view expected) {
      const auto result = parse();
      REQUIRE_FALSE(result);
      REQUIRE(result.error() == expected);
    }
  } // namespace

  TEST_CASE("Missing drum.toml") { require_error("Missing drum.toml"); }

  TEST_CASE("Invalid TOML syntax") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml", "[[unclosed");
    const auto result = parse();
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

  TEST_CASE("Non-string name value") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml",
                          "name = 42\nversion = \"0.1.0\"\ntype = \"exec\"");
    require_error("Invalid name");
  }

  TEST_CASE("Non-string version value") {
    const test_util::TestEnvironment env{};

    test_util::write_file("drum.toml",
                          "name = \"test_pkg\"\nversion = 42\ntype = \"exec\"");
    require_error("Invalid version");
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

  TEST_CASE("Valid exec manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    const auto result = parse();
    REQUIRE(result);
    const auto &manifest = result.value();
    REQUIRE(manifest.type == Manifest::Type::exec);
    REQUIRE(manifest.name == "test_pkg");
    REQUIRE(manifest.version == "0.1.0");
  }

  TEST_CASE("Valid manifest records drum.toml timestamp") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    const auto result = parse();
    REQUIRE(result);
    REQUIRE((result.value().timestamp ==
             std::filesystem::last_write_time("drum.toml")));
  }

  TEST_CASE("Valid lib manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml", "name = \"lib_pkg\"\nversion = \"0.2.0\"\ntype = \"lib\"");
    const auto result = parse();
    REQUIRE(result);
    const auto &manifest = result.value();
    REQUIRE(manifest.type == Manifest::Type::lib);
    REQUIRE(manifest.name == "lib_pkg");
    REQUIRE(manifest.version == "0.2.0");
  }

} // namespace drum::manifest::test
