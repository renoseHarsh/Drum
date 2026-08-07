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
        "test", "0.1.0", manifest::Manifest::Type::exec, future};
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
    const auto cases = {
        std::pair{manifest::Manifest::Standard::cpp11, 201103L},
        std::pair{manifest::Manifest::Standard::cpp14, 201402L},
        std::pair{manifest::Manifest::Standard::cpp17, 201703L},
        std::pair{manifest::Manifest::Standard::cpp20, 202002L},
        std::pair{manifest::Manifest::Standard::cpp23, 202302L}};
    for (const auto &[standard, version] : cases) {
      const manifest::Manifest standard_manifest{
          "test", "0.1.0", manifest::Manifest::Type::exec, {}, standard};
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
} // namespace drum::builder_cmd::compile::test
