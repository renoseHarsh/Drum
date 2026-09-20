module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_discover;

import std;

import :discover;

import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::discover::test {
  namespace {

    void create_files(const std::vector<fs::path> &paths) {
      std::ranges::for_each(paths, [](const fs::path &path) {
        fs::create_directories(path.parent_path());
        std::ofstream{path};
      });
    }

  } // namespace

  TEST_CASE("Discovers cpp and cppm files, including nested") {
    const test_util::TestEnvironment env{};

    const std::vector<fs::path> files{
        "src/main.cpp",         "src/foo.cppm",
        "src/utils/helper.cpp", "src/models/user.cppm"};
    create_files(files);

    const auto result = discover();
    REQUIRE(result);
    REQUIRE(std::ranges::is_permutation(*result, files));
  }

  TEST_CASE("Ignores non-source files") {
    const test_util::TestEnvironment env{};

    create_files({"src/main.cpp", "src/main.h", "src/config.txt",
                  "src/assets/data.bin"});

    const auto result = discover();
    REQUIRE(result);
    REQUIRE(*result == std::vector<fs::path>{"src/main.cpp"});
  }

  TEST_CASE("Empty src directory") {
    const test_util::TestEnvironment env{};

    fs::create_directory("src");
    const auto result = discover();
    REQUIRE(result->empty());
  }

} // namespace drum::builder_cmd::discover::test
