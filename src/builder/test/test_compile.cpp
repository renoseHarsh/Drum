module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_compile;

import std;

import :compile;

import test_util;
import compiler_flags;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile::test {
  namespace {
    using clock = fs::file_time_type::clock;
    const auto now = clock::now();

    const compiler_flags::Flags flags{};
    const fs::file_time_type manifest_timestamp =
        now - std::chrono::seconds{10};

    const fs::path main_src{"main.cpp"};
    const fs::path main_obj{"main.cpp.o"};

  } // namespace

  TEST_CASE("Compile failure propagates from the compiler") {
    const test_util::TestEnvironment env{};

    const auto result =
        compile({{"missing.cpp", "missing.cpp.o"}}, flags, manifest_timestamp);
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Initial build creates object and dep file") {
    const test_util::TestEnvironment env{};
    test_util::write_file("main.cpp", "int main() {}\n");

    REQUIRE(compile({{main_src, main_obj}}, flags, manifest_timestamp));

    REQUIRE(fs::exists(main_obj));
    REQUIRE(fs::exists("main.cpp.d"));
  }

  TEST_CASE("Compiles to a nested object path") {
    const test_util::TestEnvironment env{};

    test_util::write_file("bar.cpp", "void bar() {}\n");

    const auto result =
        compile({{"bar.cpp", "foo/bar.cpp.o"}}, flags, manifest_timestamp);

    REQUIRE(result);
    REQUIRE(std::ranges::equal(*result, std::array{"foo/bar.cpp.o"}));
    REQUIRE(fs::exists("foo/bar.cpp.o"));
  }

  TEST_CASE("Only the stale source recompiles") {
    const test_util::TestEnvironment env{};
    test_util::write_file(main_src, "int main() {}\n");
    test_util::write_file("utils.cpp", "void util() {}\n");

    REQUIRE(compile({{main_src, main_obj}, {"utils.cpp", "utils.cpp.o"}}, flags,
                    manifest_timestamp));

    const auto main_baseline = fs::last_write_time(main_obj);
    const auto util_baseline = fs::last_write_time("utils.cpp.o");

    fs::last_write_time("utils.cpp", now + std::chrono::seconds{10});

    REQUIRE(compile({{main_src, main_obj}, {"utils.cpp", "utils.cpp.o"}}, flags,
                    manifest_timestamp));
    REQUIRE((fs::last_write_time(main_obj) == main_baseline));
    REQUIRE((fs::last_write_time("utils.cpp.o") > util_baseline));
  }

} // namespace drum::builder_cmd::compile::test
