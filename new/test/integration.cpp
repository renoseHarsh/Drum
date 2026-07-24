module;

#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>

module new_cmd:integration;

import std;

import new_cmd;
import test_environment;

namespace drum::new_cmd::integration {
  CATCH_REGISTER_LISTENER(test_env::TestEnvironment);

  namespace {
    void check_valid_file(const std::filesystem::path &file_name) {
      REQUIRE(std::filesystem::exists(file_name));
      REQUIRE(std::filesystem::is_regular_file(file_name));
    }

    void check_valid_dir(const std::filesystem::path &path) {
      REQUIRE(std::filesystem::exists(path));
      REQUIRE(std::filesystem::is_directory(path));
    }
  }; // namespace

  TEST_CASE("Valid executable package") {
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

    constexpr std::string_view pkg_name{"existing_pkg"};
    std::filesystem::create_directory(pkg_name);

    NewArgs args{std::string{pkg_name}, NewArgs::PackageType::executable};

    auto result = execute(args);
    REQUIRE_FALSE(result.has_value());
    REQUIRE(result.error() ==
            std::format("Destination '{}' already exists", pkg_name));
  }
} // namespace drum::new_cmd::integration
