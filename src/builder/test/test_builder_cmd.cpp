#include <catch2/catch_test_macros.hpp>

import std;

import builder_cmd;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::test {

  TEST_CASE("Exec package missing src directory") {
    const test_util::TestEnvironment env{};

    const auto result = execute({}, {.type = manifest::Manifest::Type::exec});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Invalid executable package layout");
  }

  TEST_CASE("Exec package missing main.cpp") {
    const test_util::TestEnvironment env{};

    fs::create_directory("src");
    const auto result = execute({}, {.type = manifest::Manifest::Type::exec});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Invalid executable package layout");
  }

  TEST_CASE("Lib package missing include directory") {
    const test_util::TestEnvironment env{};

    fs::create_directory("src");
    const auto result =
        execute({}, {.name = "demo", .type = manifest::Manifest::Type::lib});
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
        execute({}, {.name = "demo", .type = manifest::Manifest::Type::exec});
    REQUIRE(result);
    REQUIRE(fs::exists("build/debug/demo"));
  }

  TEST_CASE("Full pipeline produces release executable") {
    const test_util::TestEnvironment env{};

    test_util::write_file(
        "drum.toml",
        "name = \"test_pkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    test_util::write_file("src/main.cpp", "int main() {}");

    const auto result =
        execute({.profile = "release"},
                {.name = "demo", .type = manifest::Manifest::Type::exec});
    REQUIRE(result);
    REQUIRE(fs::exists("build/release/demo"));
  }

  TEST_CASE("Full pipeline produces static library") {
    const test_util::TestEnvironment env{};

    fs::create_directories("src");
    fs::create_directory("include");
    test_util::write_file("src/math.cpp",
                          "int add(int a, int b) { return a + b; }\n");

    const auto result =
        execute({}, {.name = "demo", .type = manifest::Manifest::Type::lib});
    REQUIRE(result);
    REQUIRE(fs::exists("build/debug/demo.a"));
  }

  TEST_CASE("Compile failure propagates from pipeline") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() { syntax error\n");
    const auto result =
        execute({}, {.name = "demo", .type = manifest::Manifest::Type::exec});
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Nested sources are discovered and compiled") {
    const test_util::TestEnvironment env{};

    fs::create_directories("src/core");
    test_util::write_file("src/main.cpp", "int main() {}\n");
    test_util::write_file("src/core/util.cpp", "void util() {}\n");

    const auto result =
        execute({}, {.name = "demo", .type = manifest::Manifest::Type::exec});
    REQUIRE(result);
    REQUIRE(fs::exists("build/debug/main.cpp.o"));
    REQUIRE(fs::exists("build/debug/core/util.cpp.o"));
  }

  TEST_CASE("Full pipeline compiles C++23 modules") {
    test_util::write_file(
        "drum.toml", "name = \"modpkg\"\nversion = \"0.1.0\"\ntype = \"exec\"");
    test_util::write_file("src/greeter.cppm",
                          "export module greeter;\nexport void greet();");
    test_util::write_file("src/greeter.cpp",
                          "module greeter;\nvoid greet() {}");
    test_util::write_file("src/main.cpp",
                          "import greeter;\nint main() { greet(); }");

    const auto result =
        execute({}, {.name = "modpkg", .type = manifest::Manifest::Type::exec});
    REQUIRE(result);
    REQUIRE(fs::exists("build/debug/modpkg"));
  }

  TEST_CASE("Custom profile builds into its own directory") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/main.cpp", "int main() {}\n");

    auto m = manifest::Manifest{.name = "demo",
                                .type = manifest::Manifest::Type::exec};
    m.profiles["ci"] = {"-O2"};

    const auto result = execute({.profile = "ci"}, m);
    REQUIRE(result);
    REQUIRE(fs::exists("build/ci/demo"));
    REQUIRE_FALSE(fs::exists("build/debug/demo"));
  }

  TEST_CASE("Unknown profile is rejected") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/main.cpp", "int main() {}\n");

    const auto result =
        execute({.profile = "missing"},
                {.name = "demo", .type = manifest::Manifest::Type::exec});
    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Unknown profile: missing");
  }

  TEST_CASE("Build and profile flags are passed to the compiler") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/main.cpp",
                          "#ifndef DRUM_BUILD\n"
                          "#error \"build flags not applied\"\n"
                          "#endif\n"
                          "#ifndef DRUM_PROFILE\n"
                          "#error \"profile flags not applied\"\n"
                          "#endif\n"
                          "int main() {}\n");

    auto m = manifest::Manifest{.name = "demo",
                                .type = manifest::Manifest::Type::exec};
    m.build.flags = {"-DDRUM_BUILD"};
    m.profiles["ci"] = {"-DDRUM_PROFILE"};

    REQUIRE(execute({.profile = "ci"}, m));
  }

} // namespace drum::builder_cmd::test
