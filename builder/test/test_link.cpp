module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_link;

import std;

import :compile;
import :link;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::link::test {
  TEST_CASE("Links compiled objects into executable") {
    const test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() {}");
    const auto objects = compile::compile({"src/main.cpp"});
    REQUIRE(objects);

    const auto result = link::link(objects.value());
    REQUIRE(result);
    REQUIRE(fs::exists("build/main"));
  }
} // namespace drum::builder_cmd::link::test
