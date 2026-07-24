module;

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

module builder_cmd:test_validate;

import std;

import :validate;
import test_environment;

namespace drum::builder_cmd::validate::test {
  CATCH_REGISTER_LISTENER(test_env::TestEnvironment);

  namespace {
    void write_file(const std::filesystem::path &path,
                    std::string_view content) {

      std::ofstream file{path};
      file << content;
    }

    void require_error(std::string_view expected) {
      auto result = validate();
      REQUIRE_FALSE(result.has_value());
      REQUIRE(result.error() == expected);
    }

  } // namespace

  TEST_CASE("Missing drum.toml") { require_error("Missing drum.toml"); }

  TEST_CASE("Invalid TOML syntax") {
    write_file("drum.toml", "[[unclosed");
    auto result = validate();
    REQUIRE_FALSE(result.has_value());
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Missing name in manifest") {
    write_file("drum.toml", "version = \"0.1.0\"\ntype = \"exec\"");
    require_error("Missing name");
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Missing version in manifest") {
    write_file("drum.toml", "name = \"test_pkg\"\ntype = \"exec\"");
    require_error("Missing version");
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Missing type in manifest") {
    write_file("drum.toml", "name = \"test_pkg\"\nversion = \"0.1.0\"");
    require_error("Missing type");
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Non-string type value") {
    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = 42");
    require_error("Invalid type");
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Unrecognized type value") {
    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"foo\"");
    require_error("Invalid type");
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Exec package missing src directory") {
    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    require_error("Invalid executable package layout");
    std::filesystem::remove("drum.toml");
  }

  TEST_CASE("Exec package missing main.cpp") {
    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    std::filesystem::create_directory("src");
    require_error("Invalid executable package layout");
    std::filesystem::remove("drum.toml");
    std::filesystem::remove_all("src");
  }

  TEST_CASE("Valid executable package") {
    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    std::filesystem::create_directory("src");
    write_file("src/main.cpp", "int main() {}");
    auto result = validate();
    REQUIRE(result.has_value());
    std::filesystem::remove("drum.toml");
    std::filesystem::remove_all("src");
  }

  TEST_CASE("Lib type not implemented") {
    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"lib\"");
    require_error("Not implemented lib");
    std::filesystem::remove("drum.toml");
  }

} // namespace drum::builder_cmd::validate::test
