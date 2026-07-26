module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_builder_cmd;

import std;

import builder_cmd;
import test_environment;

namespace fs = std::filesystem;

namespace drum::builder_cmd::test {
  namespace {
    void write_file(const fs::path &path, std::string_view content) {
      if (path.has_parent_path()) {
        fs::create_directories(path.parent_path());
      }
      std::ofstream file{path};
      file << content;
    }
  } // namespace

  TEST_CASE("Full pipeline produces executable") {
    test_env::TestEnvironment env{};

    write_file("drum.toml",
               "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    write_file("src/main.cpp", "int main() {}");

    BuildArgs args{};
    auto result = execute(args);
    REQUIRE(result.has_value());
    REQUIRE(fs::exists("build/main"));
  }

  TEST_CASE("Missing drum.toml fails") {
    test_env::TestEnvironment env{};

    BuildArgs args{};
    auto result = execute(args);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() == "Missing drum.toml");
  }
} // namespace drum::builder_cmd::test
