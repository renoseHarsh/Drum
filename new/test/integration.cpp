module;

#include <catch2/catch_test_macros.hpp>

module new_cmd:integration;

import std;

import new_cmd;
import test_util;

namespace fs = std::filesystem;

namespace drum::new_cmd::integration {
  namespace {
    void check_valid_file(const std::filesystem::path &file_name) {
      REQUIRE(fs::exists(file_name));
      REQUIRE(fs::is_regular_file(file_name));
    }

    void check_valid_dir(const std::filesystem::path &path) {
      REQUIRE(fs::exists(path));
      REQUIRE(fs::is_directory(path));
    }
  }; // namespace

  TEST_CASE("Valid executable package") {
    test_util::TestEnvironment env;

    constexpr std::string_view pkg_name{"exec_pkg"};

    NewArgs args{std::string{pkg_name}, NewArgs::PackageType::executable};

    auto result = execute(args);
    std::filesystem::path pkg_path{pkg_name};

    check_valid_dir(pkg_path);
    check_valid_file(pkg_path / "drum.toml");

    check_valid_dir(pkg_path / "src");
    check_valid_file(pkg_path / "src" / "main.cpp");
  }

  TEST_CASE("Valid library package") {
    test_util::TestEnvironment env;

    constexpr std::string_view pkg_name{"lib_pkg"};

    NewArgs args{std::string{pkg_name}, NewArgs::PackageType::library};

    auto result = execute(args);
    std::filesystem::path pkg_path{pkg_name};

    check_valid_dir(pkg_path);
    check_valid_file(pkg_path / "drum.toml");

    check_valid_dir(pkg_path / "src");
    check_valid_dir(pkg_path / "include" / pkg_path);
  }

  TEST_CASE("Fails if destination exists") {
    test_util::TestEnvironment env;

    constexpr std::string_view pkg_name{"existing_pkg"};
    std::filesystem::create_directory(pkg_name);

    NewArgs args{std::string{pkg_name}, NewArgs::PackageType::executable};

    auto result = execute(args);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            std::format("Destination '{}' already exists", pkg_name));
  }
} // namespace drum::new_cmd::integration
