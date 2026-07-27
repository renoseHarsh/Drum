module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_builder_cmd;

import std;

import builder_cmd;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::test {
  TEST_CASE("Full pipeline produces executable") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    test_util::write_file("src/main.cpp", "int main() {}");

    const BuildArgs args{};
    const auto result = execute(args);
    REQUIRE(result);
    REQUIRE(fs::exists("build/main"));
  }

  TEST_CASE("Missing drum.toml fails") {
    const test_util::TestEnvironment env{};

    const BuildArgs args{};
    const auto result = execute(args);
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Missing drum.toml");
  }
} // namespace drum::builder_cmd::test
