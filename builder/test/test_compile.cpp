module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_compile;

import std;

import :compile;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile::test {
  TEST_CASE("Single source file") {
    test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() {}");
    auto result = compile({"src/main.cpp"});

    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 1);
    REQUIRE(result.value().front() == "build/main.o");
    REQUIRE(fs::exists("build/main.o"));
  }

  TEST_CASE("Multiple source files") {
    test_util::TestEnvironment env{};

    test_util::write_file("src/main.cpp", "int main() {}");
    test_util::write_file("src/utils.cpp", "void util() {}");
    auto result = compile({"src/main.cpp", "src/utils.cpp"});

    REQUIRE(result.has_value());
    REQUIRE(result.value().size() == 2);
    REQUIRE(fs::exists("build/main.o"));
    REQUIRE(fs::exists("build/utils.o"));
  }
} // namespace drum::builder_cmd::compile::test
