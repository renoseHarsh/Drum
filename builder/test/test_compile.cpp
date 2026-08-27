module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_compile;

import std;

import :compile;

import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile::test {
  namespace {
    constexpr manifest::Manifest manifest{"test", "0.1.0",
                                          manifest::Manifest::Type::exec};

    void setup() {
      test_util::write_file("src/main.cpp",
                            "#include \"header.h\"\nint main() {}\n");
      test_util::write_file("src/header.h", "int x;\n");
      const auto result = compile({"src/main.cpp"}, manifest);
      REQUIRE(result);
    }
    using clock = fs::file_time_type::clock;
    const fs::file_time_type future = clock::now() + std::chrono::seconds{10};
    const fs::file_time_type past = clock::now() + std::chrono::seconds{-10};

    using Warnings = manifest::Manifest::Build::Warnings;

    auto compile_with_warnings(std::string_view src, Warnings level,
                               bool errors) {
      test_util::write_file("src/main.cpp", src);
      const manifest::Manifest m{
          "test", "0.1.0", manifest::Manifest::Type::exec,
          manifest::Manifest::Build{.warnings = level,
                                    .warnings_as_errors = errors}};
      return compile({"src/main.cpp"}, m);
    }
  } // namespace

  TEST_CASE("Compile failure propagates from the compiler") {
    const test_util::TestEnvironment env{};

    const auto result = compile({"src/missing.cpp"}, manifest);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Initial build creates object and dep file") {
    const test_util::TestEnvironment env{};
    setup();

    REQUIRE(fs::exists("build/main.o"));
    REQUIRE(fs::exists("build/main.d"));
  }

  TEST_CASE("Cache hit: unchanged inputs do not recompile") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    const auto result = compile({"src/main.cpp"}, manifest);

    REQUIRE(result);
    REQUIRE((*result == std::vector<fs::path>{"build/main.o"}));
    REQUIRE((fs::last_write_time("build/main.o") == baseline));
  }

  TEST_CASE("Recompiles when manifest is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const manifest::Manifest future_manifest{
        "test", "0.1.0", manifest::Manifest::Type::exec, {}, future};
    const auto baseline = fs::last_write_time("build/main.o");
    compile({"src/main.cpp"}, future_manifest);

    REQUIRE((fs::last_write_time("build/main.o") > baseline));
  }

  TEST_CASE("Recompiles when source file is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::last_write_time("src/main.cpp", future);
    compile({"src/main.cpp"}, manifest);

    REQUIRE((fs::last_write_time("build/main.o") > baseline));
  }

  TEST_CASE("Recompiles when header dependency is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::last_write_time("src/header.h", future);
    compile({"src/main.cpp"}, manifest);

    REQUIRE((fs::last_write_time("build/main.o") > baseline));
  }

  TEST_CASE("Unrelated header change does not recompile") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/unrelated.h", "int y;\n");
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::last_write_time("src/unrelated.h", future);
    compile({"src/main.cpp"}, manifest);

    REQUIRE((fs::last_write_time("build/main.o") == baseline));
  }

  TEST_CASE("Recompiles when dependency file is missing") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::remove("build/main.d");
    compile({"src/main.cpp"}, manifest);

    REQUIRE((fs::last_write_time("build/main.o") > baseline));
    REQUIRE(fs::exists("build/main.d"));
  }

  TEST_CASE("Recompiles when dependency target mismatches object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    test_util::write_file("build/main.d", "wrong.o: src/main.cpp src/header.h");
    compile({"src/main.cpp"}, manifest);

    REQUIRE((fs::last_write_time("build/main.o") > baseline));
    REQUIRE(fs::exists("build/main.d"));
  }

  TEST_CASE("Recompiles when a listed dependency has been removed") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    test_util::write_file("src/main.cpp", "int main() {}\n");
    fs::last_write_time("src/main.cpp", past);
    fs::remove("src/header.h");

    compile({"src/main.cpp"}, manifest);
    REQUIRE((fs::last_write_time("build/main.o") > baseline));

    const auto rebuilt = fs::last_write_time("build/main.o");
    compile({"src/main.cpp"}, manifest);
    REQUIRE((fs::last_write_time("build/main.o") == rebuilt));
  }

  TEST_CASE("Compiles with the manifest standard") {
    const test_util::TestEnvironment env{};

    using enum manifest::Manifest::Build::Standard;
    constexpr std::array cases{
        std::pair{cpp11, 201103L}, std::pair{cpp14, 201402L},
        std::pair{cpp17, 201703L}, std::pair{cpp20, 202002L},
        std::pair{cpp23, 202302L}};
    for (const auto &[standard, version] : cases) {
      const manifest::Manifest standard_manifest{
          "test", "0.1.0", manifest::Manifest::Type::exec, {standard}};
      test_util::write_file(
          "src/main.cpp",
          std::format("static_assert(__cplusplus == {});\n", version));
      const auto result = compile({"src/main.cpp"}, standard_manifest);
      REQUIRE(result);
    }
  }

  TEST_CASE("Recompiles only the stale source in a multi-source build") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/main.cpp", "int main() {}\n");
    test_util::write_file("src/utils.cpp", "void util() {}\n");

    const auto result = compile({"src/main.cpp", "src/utils.cpp"}, manifest);
    REQUIRE(result);

    const auto main_baseline = fs::last_write_time("build/main.o");
    const auto util_baseline = fs::last_write_time("build/utils.o");

    fs::last_write_time("src/utils.cpp", future);

    compile({"src/main.cpp", "src/utils.cpp"}, manifest);

    REQUIRE((fs::last_write_time("build/main.o") == main_baseline));
    REQUIRE((fs::last_write_time("build/utils.o") > util_baseline));
  }

  TEST_CASE("Suppresses warnings at the none level") {
    REQUIRE(compile_with_warnings("int fn(){}", Warnings::none, false));
    REQUIRE(compile_with_warnings("int fn(){}", Warnings::none, true));
  }

  TEST_CASE("Compiles and promotes warnings at the default_ level") {
    REQUIRE(compile_with_warnings("int fn(){}", Warnings::default_, false));
    REQUIRE_FALSE(
        compile_with_warnings("int fn(){}", Warnings::default_, true));
    REQUIRE(
        compile_with_warnings("void fn(int x) {}", Warnings::default_, true));
  }

  TEST_CASE("Compiles and promotes warnings at the all level") {
    // -Wunused-parameter is enabled by -Wall/-Wextra.
    REQUIRE(compile_with_warnings("void fn(int x) {}", Warnings::all, false));
    REQUIRE_FALSE(
        compile_with_warnings("void fn(int x) {}", Warnings::all, true));

    // GNU statement expressions require -Wpedantic.
    REQUIRE(compile_with_warnings(
        "int main() {int x = ({int y = 42;y;});return x;}", Warnings::all,
        true));
  }

  TEST_CASE("Compiles and promotes warnings at the pedantic level") {
    // -Wpedantic catches the GNU extension.
    REQUIRE(compile_with_warnings(
        "int main() {int x = ({int y = 42;y;});return x;}", Warnings::pedantic,
        false));
    REQUIRE_FALSE(compile_with_warnings(
        "int main() {int x = ({int y = 42;y;});return x;}", Warnings::pedantic,
        true));

    // Valid, warning-free C++.
    REQUIRE(compile_with_warnings("int main(){int x = 42;return x;}",
                                  Warnings::pedantic, true));
  }

} // namespace drum::builder_cmd::compile::test
