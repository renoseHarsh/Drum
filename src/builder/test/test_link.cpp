module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_link;

import std;

import :link;
import :compile;

import test_util;
import compiler_flags;

namespace fs = std::filesystem;

namespace drum::builder_cmd::link::test {
  namespace {
    using clock = fs::file_time_type::clock;
    const auto now = clock::now();

    const fs::path main_bin{"main"};

    std::vector<fs::path> build_objects() {
      const compiler_flags::Flags flags{};
      const fs::file_time_type manifest_timestamp = now;

      test_util::write_file("main.cpp", "int main() {}");
      const auto objects =
          compile::compile({{"main.cpp", "main.o"}}, flags, manifest_timestamp);
      REQUIRE(objects);
      return *objects;
    }

    const fs::file_time_type future = now + std::chrono::seconds{10};
    const fs::file_time_type past = now - std::chrono::seconds{10};
  }; // namespace

  TEST_CASE("Links compiled objects into executable") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();

    REQUIRE(link(objects, main_bin));
    REQUIRE(fs::exists(main_bin));
  }

  TEST_CASE("Link failure returns unexpected") {
    const test_util::TestEnvironment env{};

    REQUIRE_FALSE(link({"nonexistent.o"}, main_bin));
  }

  TEST_CASE("Cache hit: unchanged objects do not relink") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(link(objects, main_bin));

    const auto baseline = fs::last_write_time(main_bin);
    REQUIRE(link(objects, main_bin));
    REQUIRE((fs::last_write_time(main_bin) == baseline));
  }

  TEST_CASE("Relinks when an object is newer than the executable") {
    const test_util::TestEnvironment env{};

    const auto objects = build_objects();
    REQUIRE(link(objects, main_bin));

    const auto baseline = fs::last_write_time(main_bin);
    fs::last_write_time(objects.front(), future);

    REQUIRE(link(objects, main_bin));
    REQUIRE((fs::last_write_time(main_bin) > baseline));
  }
} // namespace drum::builder_cmd::link::test
