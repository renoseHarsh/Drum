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
    constexpr manifest::Manifest manifest{"test", "0.1.0",
                                          manifest::Manifest::Type::exec};

    std::vector<fs::path> build_objects() {
      test_util::write_file("src/math.cpp",
                            "int add(int a, int b) { return a + b; }\n");
      const auto objects = compile::compile({"src/math.cpp"}, manifest);
      REQUIRE(objects);
      return objects.value();
    }
  } // namespace

  TEST_CASE("Archives compiled objects into a static library") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    const auto result = archive(objects, "libmath.a");

    REQUIRE(result);
    REQUIRE(fs::exists("build/libmath.a"));
  }

  TEST_CASE("Archive failure returns unexpected") {
    const test_util::TestEnvironment env{};

    const auto result = archive({fs::path{"build/nonexistent.o"}}, "libmath.a");
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Empty object list creates no archive") {
    const test_util::TestEnvironment env{};

    const auto result = archive({}, "libmath.a");
    REQUIRE(result);
    REQUIRE_FALSE(fs::exists("build/libmath.a"));
  }

  namespace {
    using clock = fs::file_time_type::clock;
    const fs::file_time_type past = clock::now() - std::chrono::seconds{10};
  } // namespace

  TEST_CASE("Cache hit: unchanged objects do not rearchive") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(archive(objects, "libmath.a"));

    const auto baseline = fs::last_write_time("build/libmath.a");
    REQUIRE(archive(objects, "libmath.a"));
    REQUIRE((fs::last_write_time("build/libmath.a") == baseline));
  }

  TEST_CASE("Re-archives when an object is newer than the archive") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(archive(objects, "libmath.a"));

    fs::last_write_time("build/libmath.a", past);
    REQUIRE(archive(objects, "libmath.a"));
    REQUIRE((fs::last_write_time("build/libmath.a") > past));
  }
} // namespace drum::builder_cmd::archive::test
