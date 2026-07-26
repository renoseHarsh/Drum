module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_link;

import std;

import :compile;
import :link;
import test_environment;

namespace fs = std::filesystem;

namespace drum::builder_cmd::link::test {
  namespace {
    void write_file(const fs::path &path, std::string_view content) {
      fs::create_directories(path.parent_path());
      std::ofstream file{path};
      file << content;
    }
  } // namespace

  TEST_CASE("Links compiled objects into executable") {
    test_env::TestEnvironment env{};

    write_file("src/main.cpp", "int main() {}");
    auto objects = compile::compile({"src/main.cpp"});
    REQUIRE(objects.has_value());

    auto result = link::link(objects.value());
    REQUIRE(result.has_value());
    REQUIRE(fs::exists("build/main"));
  }
} // namespace drum::builder_cmd::link::test
