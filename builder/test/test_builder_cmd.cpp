#include <catch2/catch_test_macros.hpp>

import std;

import builder_cmd;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::test {
  TEST_CASE("Exec package missing src directory") {
    const test_util::TestEnvironment env{};

    const auto result = execute({}, {.type = manifest::Type::exec});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Invalid executable package layout");
  }

  TEST_CASE("Exec package missing main.cpp") {
    const test_util::TestEnvironment env{};

    fs::create_directory("src");
    const auto result = execute({}, {.type = manifest::Type::exec});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Invalid executable package layout");
  }

  TEST_CASE("Lib package missing include directory") {
    const test_util::TestEnvironment env{};

    fs::create_directory("src");
    const auto result =
        execute({}, {.name = "demo", .type = manifest::Type::lib});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Invalid executable package layout");
  }

  TEST_CASE("Full pipeline produces executable") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    test_util::write_file("src/main.cpp", "int main() {}");

    const auto result =
        execute({}, {.name = "demo", .type = manifest::Type::exec});
    REQUIRE(result);
    REQUIRE(fs::exists("build/demo"));
  }

  TEST_CASE("Full pipeline produces static library") {
    const test_util::TestEnvironment env{};

    fs::create_directories("src");
    fs::create_directory("include");
    test_util::write_file("src/math.cpp",
                          "int add(int a, int b) { return a + b; }\n");

    const auto result =
        execute({}, {.name = "demo", .type = manifest::Type::lib});
    REQUIRE(result);
    REQUIRE(fs::exists("build/demo"));
  }

  TEST_CASE("Compile failure propagates from pipeline") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() { syntax error\n");
    const auto result =
        execute({}, {.name = "demo", .type = manifest::Type::exec});
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Nested sources are discovered and compiled") {
    const test_util::TestEnvironment env{};

    fs::create_directories("src/core");
    test_util::write_file("src/main.cpp", "int main() {}\n");
    test_util::write_file("src/core/util.cpp", "void util() {}\n");

    const auto result =
        execute({}, {.name = "demo", .type = manifest::Type::exec});
    REQUIRE(result);
    REQUIRE(fs::exists("build/main.o"));
    REQUIRE(fs::exists("build/core/util.o"));
  }
} // namespace drum::builder_cmd::test
