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
    constexpr manifest::Manifest manifest{"test", "0.1.0",
                                          manifest::Manifest::Type::exec};
    using clock = fs::file_time_type::clock;
    const fs::file_time_type past = clock::now() - std::chrono::seconds{10};
  }; // namespace
  TEST_CASE("Link failure returns unexpected") {
    const test_util::TestEnvironment env{};

    const auto result = link::link({fs::path{"build/nonexistent.o"}}, "main");
    REQUIRE_FALSE(result);
  }

  TEST_CASE("Links compiled objects into executable") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() {}");
    const auto objects = compile::compile({"src/main.cpp"}, manifest);
    REQUIRE(objects);

    const auto result = link::link(objects.value(), "main");
    REQUIRE(result);
    REQUIRE(fs::exists("build/main"));
  }

  TEST_CASE("Cache hit: unchanged objects do not relink") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() {}");
    const auto objects = compile::compile({"src/main.cpp"}, manifest);
    REQUIRE(objects);
    REQUIRE(link::link(objects.value(), "main"));

    const auto baseline = fs::last_write_time("build/main");

    REQUIRE(link::link(objects.value(), "main"));
    REQUIRE((fs::last_write_time("build/main") == baseline));
  }

  TEST_CASE("Relinks when an object is newer than the executable") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() {}");
    const auto objects = compile::compile({"src/main.cpp"}, manifest);
    REQUIRE(objects);
    REQUIRE(link::link(objects.value(), "main"));

    fs::last_write_time("build/main", past);
    REQUIRE(link::link(objects.value(), "main"));
    REQUIRE((fs::last_write_time("build/main") > past));
  }
} // namespace drum::builder_cmd::link::test
