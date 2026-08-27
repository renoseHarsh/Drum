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

  TEST_CASE("Invalid warnings") {
    const test_util::TestEnvironment env{};
    require_parse_error(
        std::format("{}[build]\nwarnings = \"foo\"", base_manifest),
        "Invalid warnings level");
    require_parse_error(std::format("{}[build]\nwarnings = 42", base_manifest),
                        "Invalid warnings");
  }

  TEST_CASE("Invalid warnings_as_errors") {
    const test_util::TestEnvironment env{};
    require_parse_error(
        std::format("{}[build]\nwarnings_as_errors = \"foo\"", base_manifest),
        "Invalid warnings_as_errors");
    require_parse_error(
        std::format("{}[build]\nwarnings_as_errors = 42", base_manifest),
        "Invalid warnings_as_errors");
  }

  TEST_CASE("Invalid extra_flags") {
    const test_util::TestEnvironment env{};
    require_parse_error(
        std::format("{}[build]\nextra_flags = \"-O2\"", base_manifest),
        "Invalid extra_flags");
    require_parse_error(
        std::format("{}[build]\nextra_flags = [\"-O2\", 42]", base_manifest),
        "Invalid extra_flags");
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
    REQUIRE(
        (result->timestamp == std::filesystem::last_write_time("drum.toml")));
    REQUIRE(result->build.standard == Manifest::Build::Standard::cpp23);
    REQUIRE(result->build.warnings == Manifest::Build::Warnings::default_);
    REQUIRE_FALSE(result->build.warnings_as_errors);
    REQUIRE(result->build.extra_flags.empty());
  }

  TEST_CASE("Valid lib manifest") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml", "name = \"lib_pkg\"\nversion = \"0.2.0\"\ntype = \"lib\"");
    const auto result = parse();
    REQUIRE(result);
    REQUIRE(result->name == "lib_pkg");
    REQUIRE(result->version == "0.2.0");
    REQUIRE(result->type == Manifest::Type::lib);
    REQUIRE_FALSE(result->build.warnings_as_errors);
    REQUIRE(result->build.extra_flags.empty());
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

  TEST_CASE("Warnings parsed from build table") {
    const test_util::TestEnvironment env{};

    using enum manifest::Manifest::Build::Warnings;
    const auto cases = {std::pair{"none", none}, std::pair{"default", default_},
                        std::pair{"all", all}, std::pair{"pedantic", pedantic}};
    for (const auto &[warnings, expected] : cases) {
      test_util::write_file(
          "drum.toml",
          std::format("{}[build]\nwarnings = \"{}\"", base_manifest, warnings));

      const auto result = parse();
      REQUIRE(result);
      REQUIRE(result->build.warnings == expected);
    }
  }

  TEST_CASE("Warnings_as_errors parsed from build table") {
    const test_util::TestEnvironment env{};

    const auto cases = {std::pair{true, true}, std::pair{false, false}};
    for (const auto &[value, expected] : cases) {
      test_util::write_file("drum.toml",
                            std::format("{}[build]\nwarnings_as_errors = {}",
                                        base_manifest, value));

      const auto result = parse();
      REQUIRE(result);
      REQUIRE(result->build.warnings_as_errors == expected);
    }
  }

  TEST_CASE("Extra_flags parsed from build table") {
    const test_util::TestEnvironment env{};

    std::vector<std::string> extra_flags{"-fno-rtti", "-fvisibility=hidden"};
    std::string flags = extra_flags |
                        std::views::transform([](std::string_view flag) {
                          return std::format("\"{}\"", flag);
                        }) |
                        std::views::join_with(std::string_view{", "}) |
                        std::ranges::to<std::string>();

    test_util::write_file(
        "drum.toml",
        std::format("{}[build]\nextra_flags = [{}]", base_manifest, flags));

    const auto result = parse();
    REQUIRE(result);
    REQUIRE(result->build.extra_flags == extra_flags);
  }
} // namespace drum::manifest::test
