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

    void setup() {
      test_util::write_file("src/main.cpp",
                            "#include \"header.h\"\nint main() {}\n");
      test_util::write_file("src/header.h", "int x;\n");
      const auto result =
          compile({"src/main.cpp"}, compiler, manifest_timestamp);
      REQUIRE(result);
    }

    const fs::file_time_type future = now + std::chrono::seconds{10};
    const fs::file_time_type past = now + std::chrono::seconds{-10};
  } // namespace

  TEST_CASE("Compile failure propagates from the compiler") {
    const test_util::TestEnvironment env{};

    const auto result =
        compile({"src/missing.cpp"}, compiler, manifest_timestamp);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Initial build creates object and dep file") {
    const test_util::TestEnvironment env{};
    setup();

    REQUIRE(fs::exists("build/main.o"));
    REQUIRE(fs::exists("build/main.d"));
  }

  TEST_CASE("Preserves source directory structure in object path") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/foo/bar.cpp", "void bar() {}\n");

    const auto result =
        compile({"src/foo/bar.cpp"}, compiler, manifest_timestamp);

    REQUIRE(result);
    REQUIRE(*result == std::vector<fs::path>{"build/foo/bar.o"});
    REQUIRE(fs::exists("build/foo/bar.o"));
  }

  TEST_CASE("Cache hit: unchanged inputs do not recompile") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    const auto result = compile({"src/main.cpp"}, compiler, manifest_timestamp);

    REQUIRE(result);
    REQUIRE((*result == std::vector<fs::path>{"build/main.o"}));
    REQUIRE((fs::last_write_time("build/main.o") == baseline));
  }

  TEST_CASE("Recompiles when manifest is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");

    REQUIRE(compile({"src/main.cpp"}, compiler, future));
    REQUIRE((fs::last_write_time("build/main.o") > baseline));
  }

  TEST_CASE("Recompiles when source file is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::last_write_time("src/main.cpp", future);

    REQUIRE(compile({"src/main.cpp"}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time("build/main.o") > baseline));
  }

  TEST_CASE("Recompiles when header dependency is newer than object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::last_write_time("src/header.h", future);

    REQUIRE(compile({"src/main.cpp"}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time("build/main.o") > baseline));
  }

  TEST_CASE("Unrelated header change does not recompile") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/unrelated.h", "int y;\n");
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::last_write_time("src/unrelated.h", future);

    REQUIRE(compile({"src/main.cpp"}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time("build/main.o") == baseline));
  }

  TEST_CASE("Recompiles when dependency file is missing") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    fs::remove("build/main.d");

    REQUIRE(compile({"src/main.cpp"}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time("build/main.o") > baseline));
    REQUIRE(fs::exists("build/main.d"));
  }

  TEST_CASE("Recompiles when dependency target mismatches object") {
    const test_util::TestEnvironment env{};
    setup();

    const auto baseline = fs::last_write_time("build/main.o");
    test_util::write_file("build/main.d", "wrong.o: src/main.cpp src/header.h");

    REQUIRE(compile({"src/main.cpp"}, compiler, manifest_timestamp));
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

    REQUIRE(compile({"src/main.cpp"}, compiler, manifest_timestamp));
    REQUIRE((fs::last_write_time("build/main.o") > baseline));

    const auto rebuilt = fs::last_write_time("build/main.o");
    compile({"src/main.cpp"}, compiler, manifest_timestamp);
    REQUIRE((fs::last_write_time("build/main.o") == rebuilt));
  }

  TEST_CASE("Recompiles only the stale source in a multi-source build") {
    const test_util::TestEnvironment env{};
    test_util::write_file("src/main.cpp", "int main() {}\n");
    test_util::write_file("src/utils.cpp", "void util() {}\n");

    REQUIRE(compile({"src/main.cpp", "src/utils.cpp"}, compiler,
                    manifest_timestamp));

    const auto main_baseline = fs::last_write_time("build/main.o");
    const auto util_baseline = fs::last_write_time("build/utils.o");

    fs::last_write_time("src/utils.cpp", future);

    REQUIRE(compile({"src/main.cpp", "src/utils.cpp"}, compiler,
                    manifest_timestamp));
    REQUIRE((fs::last_write_time("build/main.o") == main_baseline));
    REQUIRE((fs::last_write_time("build/utils.o") > util_baseline));
  }
} // namespace drum::builder_cmd::compile::test
