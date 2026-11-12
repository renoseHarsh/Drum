module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_cache;

import std;

import :cache;

import compile_unit;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::cache::test {

  namespace {

    using clock = fs::file_time_type::clock;

    const auto source_time = clock::now() - std::chrono::seconds{100};
    const auto object_time = clock::now() - std::chrono::seconds{50};
    const auto changed_time = clock::now();

    const fs::path source{"main.cpp"};
    const fs::path object{"main.cpp.o"};
    const fs::path depfile{"main.cpp.d"};

    void write_at(const fs::path &path, fs::file_time_type time,
                  std::string_view content = "") {
      test_util::write_file(path, content);
      fs::last_write_time(path, time);
    }

    compile_unit::TranslationUnit fresh_unit() {
      write_at(source, source_time);
      write_at(object, object_time);
      write_at(depfile, object_time, "main.cpp.o: main.cpp");

      return {.source = source, .object = object};
    }

    compile_unit::TranslationUnit fresh_module_unit() {
      write_at("alpha.cppm", source_time);
      write_at("alpha.cppm.o", object_time);
      write_at("alpha.cppm.pcm", object_time);
      write_at("alpha.cppm.d", object_time, "alpha.cppm.o: alpha.cppm");

      return {.source = "alpha.cppm", .object = "alpha.cppm.o"};
    }

    compile_unit::TranslationUnit importing_unit() {
      auto unit = fresh_unit();
      write_at("strings.cppm.pcm", object_time);

      unit.dependency_bmis = {
          {.logical_name = "util.strings", .path = "strings.cppm.pcm"}};

      return unit;
    }

  } // namespace

  TEST_CASE("Unit object freshness") {
    const test_util::TestEnvironment env{};

    SECTION("fresh unit is not stale") {
      REQUIRE_FALSE(unit_is_stale(fresh_unit(), source_time));
    }

    SECTION("missing object") {
      const auto unit = fresh_unit();
      fs::remove(object);
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("source newer than the object") {
      const auto unit = fresh_unit();
      fs::last_write_time(source, changed_time);
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("manifest newer than the object") {
      const auto unit = fresh_unit();
      REQUIRE(unit_is_stale(unit, changed_time));
    }
  }

  TEST_CASE("Unit depfile freshness") {
    const test_util::TestEnvironment env{};

    SECTION("missing depfile") {
      const auto unit = fresh_unit();
      fs::remove(depfile);
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("depfile target mismatches the object") {
      const auto unit = fresh_unit();
      write_at(depfile, object_time, "other.cpp.o: main.cpp");
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("listed dependency newer than the object") {
      const auto unit = fresh_unit();
      write_at("header.h", changed_time);
      write_at(depfile, object_time, "main.cpp.o: main.cpp header.h");
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("listed dependency is missing") {
      const auto unit = fresh_unit();
      write_at(depfile, object_time, "main.cpp.o: main.cpp missing.h");
      REQUIRE(unit_is_stale(unit, source_time));
    }
  }

  TEST_CASE("Module unit freshness") {
    const test_util::TestEnvironment env{};

    SECTION("fresh module unit is not stale") {
      REQUIRE_FALSE(unit_is_stale(fresh_module_unit(), source_time));
    }

    SECTION("own BMI is missing") {
      const auto unit = fresh_module_unit();
      fs::remove("alpha.cppm.pcm");
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("imported BMI older than the object") {
      REQUIRE_FALSE(unit_is_stale(importing_unit(), source_time));
    }

    SECTION("imported BMI newer than the object") {
      const auto unit = importing_unit();
      write_at("strings.cppm.pcm", changed_time);
      REQUIRE(unit_is_stale(unit, source_time));
    }

    SECTION("imported BMI is missing") {
      const auto units = importing_unit();
      fs::remove("strings.cppm.pcm");
      REQUIRE(unit_is_stale(units, source_time));
    }

    SECTION("one stale import among several") {
      auto unit = importing_unit();
      unit.dependency_bmis.push_back(
          {.logical_name = "util.bytes", .path = "bytes.cppm.pcm"});
      write_at("bytes.cppm.pcm", changed_time);
      REQUIRE(unit_is_stale(unit, source_time));
    }
  }

  TEST_CASE("Output freshness") {
    const test_util::TestEnvironment env{};

    SECTION("missing output") {
      REQUIRE(output_is_stale("app", {"main.cpp.o"}));
    }

    SECTION("output newer than its inputs") {
      write_at("app", object_time);
      write_at("main.cpp.o", source_time);
      REQUIRE_FALSE(output_is_stale("app", {"main.cpp.o"}));
    }

    SECTION("input newer than the output") {
      write_at("app", source_time);
      write_at("main.cpp.o", object_time);
      REQUIRE(output_is_stale("app", {"main.cpp.o"}));
    }

    SECTION("input is missing") {
      write_at("app", object_time);
      REQUIRE(output_is_stale("app", {"missing.o"}));
    }
  }

  TEST_CASE("Creates the build directory") {
    const test_util::TestEnvironment env{};
    REQUIRE(ensure_build_dir());
    REQUIRE(fs::is_directory("build"));
  }

} // namespace drum::builder_cmd::cache::test
