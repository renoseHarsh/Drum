module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_discover;

import std;

import :discover;
import test_util;

namespace fs = std::filesystem;

namespace drum::builder_cmd::discover::test {
  namespace {

    void require_discover(const std::vector<fs::path> &paths) {
      const auto result = discover();
      const auto &src = result.value();

      auto cpp = paths | std::views::filter([](const fs::path &path) {
                   return path.extension() == ".cpp";
                 });
      auto non_cpp = paths | std::views::filter([](const fs::path &path) {
                       return path.extension() != ".cpp";
                     });

      REQUIRE(std::ranges::all_of(cpp, [&](const fs::path &p) {
        return std::ranges::contains(src, p);
      }));

      REQUIRE(std::ranges::none_of(non_cpp, [&](const fs::path &p) {
        return std::ranges::contains(src, p);
      }));
    }

    void create_structure(const std::vector<fs::path> &paths) {
      const test_util::TestEnvironment env{};

      fs::create_directory("src");

      std::ranges::for_each(paths, [](const fs::path &path) {
        fs::create_directories(path.parent_path());
        std::ofstream{path};
      });

      require_discover(paths);
    }

  } // namespace

  TEST_CASE("Single cpp file") { create_structure({"src/main.cpp"}); }

  TEST_CASE("Nested cpp files") {
    create_structure({"src/main.cpp", "src/utils/helper.cpp"});
  }

  TEST_CASE("Ignores non-cpp files") {
    create_structure({"src/main.cpp", "src/main.h", "src/config.txt"});
  }

  TEST_CASE("Empty src directory") {
    test_util::TestEnvironment env{};

    fs::create_directory("src");
    const auto result = discover();
    REQUIRE(result.value().empty());
  }

} // namespace drum::builder_cmd::discover::test
