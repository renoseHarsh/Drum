module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_depfile;

import std;

import :depfile;

import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::depfile::test {
  namespace {
    void check_deps(std::string_view content, std::string_view target,
                    std::vector<fs::path> deps) {
      test_util::write_file("main.d", content);
      const auto result = parse("main.d");
      REQUIRE(result);

      const auto &[got_target, got_deps] = *result;
      REQUIRE(got_target == target);
      REQUIRE(got_deps == deps);
    }
  } // namespace

  TEST_CASE("Missing file returns nullopt") {
    const test_util::TestEnvironment env{};
    REQUIRE_FALSE(parse("missing.d"));
  }

  TEST_CASE("Empty file returns nullopt") {
    const test_util::TestEnvironment env{};
    test_util::write_file("main.d", "");
    REQUIRE_FALSE(parse("main.d"));
  }

  TEST_CASE("No colon returns nullopt") {
    const test_util::TestEnvironment env{};
    test_util::write_file("main.d", "just some text");
    REQUIRE_FALSE(parse("main.d"));
  }

  TEST_CASE("Single dependency") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o: src/main.cpp", "build/main.o", {"src/main.cpp"});
  }

  TEST_CASE("Multiple dependencies on one line") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o: src/main.cpp src/util.cpp src/lib.h",
               "build/main.o", {"src/main.cpp", "src/util.cpp", "src/lib.h"});
  }

  TEST_CASE("Dependencies across multiple continuation lines") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o: src/main.cpp \\\n"
               "src/util.cpp \\\n"
               "src/helper.h",
               "build/main.o",
               {"src/main.cpp", "src/util.cpp", "src/helper.h"});
  }

  TEST_CASE("Empty dependency list") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o:", "build/main.o", {});
  }

  TEST_CASE("Tab-separated dependencies") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o:\tsrc/main.cpp\tsrc/util.cpp", "build/main.o",
               {"src/main.cpp", "src/util.cpp"});
  }

  TEST_CASE("CRLF line endings") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o: src/main.cpp\r\nsrc/util.cpp", "build/main.o",
               {"src/main.cpp", "src/util.cpp"});
  }

  TEST_CASE("Consecutive spaces produce no empty entries") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o:  src/main.cpp   src/util.cpp ", "build/main.o",
               {"src/main.cpp", "src/util.cpp"});
  }

  TEST_CASE("Leading and trailing whitespace around deps") {
    const test_util::TestEnvironment env{};
    check_deps("build/main.o:  src/main.cpp  src/util.cpp  ", "build/main.o",
               {"src/main.cpp", "src/util.cpp"});
  }

  TEST_CASE("Target trims leading and trailing whitespace") {
    const test_util::TestEnvironment env{};
    check_deps("  main.o : src/main.cpp  ", "main.o", {"src/main.cpp"});
  }
}; // namespace drum::builder_cmd::depfile::test
