module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_archive;

import std;

import :archive;
import :compile;

import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::archive::test {
  namespace {
    using clock = fs::file_time_type::clock;
    const auto now = clock::now();

    const fs::path math_lib{"math.a"};

    std::vector<fs::path> build_objects() {
      const compiler::Compiler compiler{};
      const fs::file_time_type manifest_timestamp = now;

      test_util::write_file("math.cpp",
                            "int add(int a, int b) { return a + b; }");
      const auto objects = compile::compile({{"math.cpp", "math.o"}}, compiler,
                                            manifest_timestamp);
      REQUIRE(objects);
      return *objects;
    }

    const fs::file_time_type future = now + std::chrono::seconds{10};
    const fs::file_time_type past = now - std::chrono::seconds{10};
  } // namespace

  TEST_CASE("Archives compiled objects into a static library") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    const auto result = archive(objects, math_lib);

    REQUIRE(result);
    REQUIRE(fs::exists(math_lib));
  }

  TEST_CASE("Archive failure returns unexpected") {
    const test_util::TestEnvironment env{};

    REQUIRE_FALSE(archive({"nonexistent.o"}, math_lib));
  }

  TEST_CASE("Empty object list creates no archive") {
    const test_util::TestEnvironment env{};

    REQUIRE(archive({}, math_lib));
    REQUIRE_FALSE(fs::exists(math_lib));
  }

  TEST_CASE("Cache hit: unchanged objects do not rearchive") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(archive(objects, math_lib));

    const auto baseline = fs::last_write_time(math_lib);
    REQUIRE(archive(objects, math_lib));
    REQUIRE((fs::last_write_time(math_lib) == baseline));
  }

  TEST_CASE("Re-archives when an object is newer than the archive") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(archive(objects, math_lib));

    const auto baseline = fs::last_write_time(math_lib);
    fs::last_write_time(objects.front(), future);

    REQUIRE(archive(objects, math_lib));
    REQUIRE((fs::last_write_time(math_lib) > baseline));
  }
} // namespace drum::builder_cmd::archive::test
