module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_compile;

import std;

import :compile;
import :compiler;

import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile::test {
  namespace {
    using clock = fs::file_time_type::clock;
    const auto now = clock::now();

    const compiler::Compiler compiler{};
    const fs::file_time_type manifest_timestamp =
        now - std::chrono::seconds{10};

    const fs::path main_src{"main.cpp"};
    const fs::path main_obj{"main.o"};
    const fs::path main_dep{"main.d"};

    void setup() {
      test_util::write_file("main.cpp",
                            "#include \"header.h\"\nint main() {}\n");
      test_util::write_file("header.h", "int x;\n");

      const auto result =
          compile({{main_src, main_obj}}, compiler, manifest_timestamp);
      REQUIRE(result);
    }

    const fs::file_time_type future = now + std::chrono::seconds{10};
    const fs::file_time_type past = now + std::chrono::seconds{-10};
  } // namespace

  TEST_CASE("Compile failure propagates from the compiler") {
    const test_util::TestEnvironment env{};

    const auto result =
        compile({{"missing.cpp", "missing.o"}}, compiler, manifest_timestamp);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Initial build creates object and dep file") {
    const test_util::TestEnvironment env{};
    setup();

    REQUIRE(fs::exists(main_obj));
    REQUIRE(fs::exists(main_dep));
  }

  TEST_CASE("Compiles to a nested object path") {
    const test_util::TestEnvironment env{};

    test_util::write_file("bar.cpp", "void bar() {}\n");

    const auto result =
        compile({{"bar.cpp", "foo/bar.o"}}, compiler, manifest_timestamp);

    REQUIRE(result);
    REQUIRE(std::ranges::equal(*result, std::array{"foo/bar.o"}));
    REQUIRE(fs::exists("foo/bar.o"));
  }

  TEST_CASE("Cache hit: unchanged inputs do not recompile") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) == baseline));
  }

  TEST_CASE("Recompiles when manifest is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);

    REQUIRE(compile({{main_src, main_obj}}, compiler, future));
    REQUIRE((fs::last_write_time(main_obj) > baseline));
  }

  TEST_CASE("Recompiles when source file is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);
    fs::last_write_time(main_src, future);

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) > baseline));
  }

  TEST_CASE("Recompiles when header dependency is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);
    fs::last_write_time("header.h", future);

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) > baseline));
  }

  TEST_CASE("Unrelated header change does not recompile") {
    const test_util::TestEnvironment env{};
    test_util::write_file("unrelated.h", "int y;\n");
    setup();

    const auto baseline = fs::last_write_time(main_obj);
    fs::last_write_time("unrelated.h", future);

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) == baseline));
  }

  TEST_CASE("Recompiles when dependency file is missing") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);
    fs::remove(main_dep);

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) > baseline));
    REQUIRE(fs::exists(main_dep));
  }

  TEST_CASE("Recompiles when dependency target mismatches object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);
    test_util::write_file(main_dep, "wrong.o: src/main.cpp src/header.h");

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) > baseline));
    REQUIRE(fs::exists(main_dep));
  }

  TEST_CASE("Recompiles when a listed dependency has been removed") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time(main_obj);
    test_util::write_file(main_src, "int main() {}\n");
    fs::last_write_time(main_src, past);
    fs::remove("header.h");

    REQUIRE(compile({{main_src, main_obj}}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) > baseline));
  }

  TEST_CASE("Recompiles only the stale source in a multi-source build") {
    const test_util::TestEnvironment env{};
    test_util::write_file(main_src, "int main() {}\n");
    test_util::write_file("utils.cpp", "void util() {}\n");

    REQUIRE(compile({{main_src, main_obj}, {"utils.cpp", "utils.o"}}, compiler,
                    manifest_timestamp));

    const auto main_baseline = fs::last_write_time(main_obj);
    const auto util_baseline = fs::last_write_time("utils.o");

    fs::last_write_time("utils.cpp", future);

    REQUIRE(compile({{main_src, main_obj}, {"utils.cpp", "utils.o"}}, compiler,
                    manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) == main_baseline));
    REQUIRE((fs::last_write_time("utils.o") > util_baseline));
  }
} // namespace drum::builder_cmd::compile::test
