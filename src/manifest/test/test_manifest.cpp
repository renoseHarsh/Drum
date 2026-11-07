#include <catch2/catch_test_macros.hpp>

import std;

import manifest;
import test_util;

namespace drum::manifest::test {
  namespace {
    constexpr std::string_view base_manifest{
        R"(name = "test_pkg"
version = "0.1.0"
type = "exec"
)"};

    void require_parse_error(std::string_view toml, std::string_view expected) {
      test_util::write_file("drum.toml", toml);
      const auto result = parse();
      REQUIRE_FALSE(result);
      REQUIRE(result.error() == expected);
    }
  } // namespace

  TEST_CASE("Missing drum.toml") {
    const test_util::TestEnvironment env{};
    const auto result = parse();
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Missing drum.toml");
  }

  TEST_CASE("Invalid TOML syntax") {
    const test_util::TestEnvironment env{};
    test_util::write_file("drum.toml", "[[unclosed");
    REQUIRE_FALSE(parse());
  }

  TEST_CASE("Invalid name") {
    const test_util::TestEnvironment env{};
    require_parse_error("version = \"0.1.0\"\ntype = \"exec\"", "Missing name");
    require_parse_error("name = 42\nversion = \"0.1.0\"\ntype = \"exec\"",
                        "Invalid name");
  }

  TEST_CASE("Invalid version") {
    const test_util::TestEnvironment env{};
    require_parse_error("name = \"test_pkg\"\ntype = \"exec\"",
                        "Missing version");
    require_parse_error("name = \"test_pkg\"\nversion = 42\ntype = \"exec\"",
                        "Invalid version");
  }

  TEST_CASE("Invalid type") {
    const test_util::TestEnvironment env{};
    require_parse_error("name = \"test_pkg\"\nversion = \"0.1.0\"",
                        "Missing type");
    require_parse_error("name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = 42",
                        "Invalid type");
    require_parse_error(
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"foo\"",
        "Invalid type");
  }

  TEST_CASE("Invalid standard") {
    const test_util::TestEnvironment env{};
    require_parse_error(
        std::format("{}[build]\nstandard = \"c++99\"", base_manifest),
        "Invalid standard");
    require_parse_error(std::format("{}[build]\nstandard = 42", base_manifest),
                        "Invalid standard");
  }

  TEST_CASE("Invalid flag type") {
    const test_util::TestEnvironment env{};
    require_parse_error(
        std::format("{}[build]\nflags = [\"-Wall\", 42]", base_manifest),
        "Invalid flags");
  }

  TEST_CASE("Invalid profiles") {
    const test_util::TestEnvironment env{};

    SECTION("Profile is not an array") {
      require_parse_error(
          std::format("{}[profiles]\ndebug = \"-O0\"", base_manifest),
          "Invalid debug");
    }

    SECTION("Profile contains non-string values") {
      require_parse_error(
          std::format("{}[profiles]\ndebug = [\"-O0\", 42]", base_manifest),
          "Invalid debug");
    }
  }

  TEST_CASE("Valid exec manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    const auto result = parse();
    REQUIRE(result);
    REQUIRE(result->name == "test_pkg");
    REQUIRE(result->version == "0.1.0");
    REQUIRE(result->type == Manifest::Type::exec);
    REQUIRE(result->build.standard == Manifest::Build::Standard::cpp23);
    REQUIRE(result->build.flags.empty());
    REQUIRE(std::ranges::equal(result->profiles.at("debug"),
                               std::array{"-O0", "-g"}));
    REQUIRE(std::ranges::equal(result->profiles.at("release"),
                               std::array{"-O3", "-DNDEBUG"}));
    REQUIRE(
        (result->timestamp == std::filesystem::last_write_time("drum.toml")));
  }

  TEST_CASE("Valid lib manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml", "name = \"lib_pkg\"\nversion = \"0.2.0\"\ntype = \"lib\"");
    const auto result = parse();
    REQUIRE(result);
    REQUIRE(result->name == "lib_pkg");
    REQUIRE(result->version == "0.2.0");
    REQUIRE(result->build.standard == Manifest::Build::Standard::cpp23);
    REQUIRE(result->build.flags.empty());
    REQUIRE(std::ranges::equal(result->profiles.at("debug"),
                               std::array{"-O0", "-g"}));
    REQUIRE(std::ranges::equal(result->profiles.at("release"),
                               std::array{"-O3", "-DNDEBUG"}));
    REQUIRE(
        (result->timestamp == std::filesystem::last_write_time("drum.toml")));
  }

  TEST_CASE("Standard parsed from build table") {
    const test_util::TestEnvironment env{};

    using enum manifest::Manifest::Build::Standard;
    const auto cases = {std::pair{"c++11", cpp11}, std::pair{"c++14", cpp14},
                        std::pair{"c++17", cpp17}, std::pair{"c++20", cpp20},
                        std::pair{"c++23", cpp23}, std::pair{"c++26", cpp26}};
    for (const auto &[standard, expected] : cases) {
      test_util::write_file(
          "drum.toml",
          std::format("{}[build]\nstandard = \"{}\"", base_manifest, standard));

      const auto result = parse();
      REQUIRE(result);
      REQUIRE(result->build.standard == expected);
    }
  }

  TEST_CASE("Multiple flags") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        std::format("{}[build]\nflags = [\"-Wall\", \"-Wextra\", \"-Werror\"]",
                    base_manifest));
    const auto result = parse();
    REQUIRE(result);
    REQUIRE(result->build.flags ==
            std::vector<std::string>{"-Wall", "-Wextra", "-Werror"});
  }

  TEST_CASE("Profiles parsed from manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        std::format("{}[profiles]\n"
                    "debug = [\"-O1\", \"-g\"]\n"
                    "production = [\"-O3\", \"-DNDEBUG\", \"-flto\"]",
                    base_manifest));

    const auto result = parse();
    REQUIRE(result);

    REQUIRE(std::ranges::equal(result->profiles.at("debug"),
                               std::array{"-O1", "-g"}));

    REQUIRE(std::ranges::equal(result->profiles.at("release"),
                               std::array{"-O3", "-DNDEBUG"}));

    REQUIRE(std::ranges::equal(result->profiles.at("production"),
                               std::array{"-O3", "-DNDEBUG", "-flto"}));
  }

} // namespace drum::manifest::test
