module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_link;

import std;

import :compile;
import :link;

import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::link::test {
  namespace {
    using clock = fs::file_time_type::clock;

    const compiler::Compiler compiler{};
    const fs::file_time_type manifest_timestamp = clock::now();

    std::vector<fs::path> build_objects() {
      test_util::write_file("src/math.cpp", "int main() {}");
      const auto objects =
          compile::compile({"src/math.cpp"}, compiler, manifest_timestamp);
      REQUIRE(objects);
      return *objects;
    }

    const fs::file_time_type past = clock::now() - std::chrono::seconds{10};
  }; // namespace

  TEST_CASE("Links compiled objects into executable") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();

    REQUIRE(link::link(objects, "main"));
    REQUIRE(fs::exists("build/main"));
  }

  TEST_CASE("Link failure returns unexpected") {
    const test_util::TestEnvironment env{};

    REQUIRE_FALSE(link::link({fs::path{"build/nonexistent.o"}}, "main"));
  }

  TEST_CASE("Cache hit: unchanged objects do not relink") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(link::link(objects, "main"));

    const auto baseline = fs::last_write_time("build/main");
    REQUIRE(link::link(objects, "main"));
    REQUIRE((fs::last_write_time("build/main") == baseline));
  }

  TEST_CASE("Relinks when an object is newer than the executable") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(link::link(objects, "main"));

    fs::last_write_time("build/main", past);
    REQUIRE(link::link(objects, "main"));
    REQUIRE((fs::last_write_time("build/main") > past));
  }
} // namespace drum::builder_cmd::link::test
