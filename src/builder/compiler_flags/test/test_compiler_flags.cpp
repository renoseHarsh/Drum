#include <catch2/catch_test_macros.hpp>

import std;

import compiler_flags;

import test_util;

using namespace std::literals::string_view_literals;

namespace drum::compiler_flags::test {
  namespace {
    void require_compiler_args(const Flags &flags,
                               std::span<const std::string_view> expected) {
      const auto args = flags.args();

      REQUIRE(std::ranges::equal(args | std::views::drop(1), expected));
    }
  } // namespace

  TEST_CASE("Dependency generation flag is emitted") {
    Flags flags{};

    REQUIRE(std::ranges::equal(flags.args(), std::array{"-MMD"}));
  }

  TEST_CASE("Include directory appears in args") {
    Flags flags{};

    flags.add_include_directory("include");
    require_compiler_args(flags, std::array{"-Iinclude"sv});
  }

  TEST_CASE("Standard flag is emitted") {
    using enum manifest::Manifest::Build::Standard;

    constexpr std::array cases{
        std::pair{cpp11, "-std=c++11"sv}, std::pair{cpp14, "-std=c++14"sv},
        std::pair{cpp17, "-std=c++17"sv}, std::pair{cpp20, "-std=c++20"sv},
        std::pair{cpp23, "-std=c++23"sv}, std::pair{cpp26, "-std=c++26"sv},
    };

    for (const auto &[standard, arg] : cases) {
      Flags flags{};

      flags.set_standard(standard);
      require_compiler_args(flags, std::array{arg});
    }
  }

  TEST_CASE("Warning flags are emitted") {
    using enum manifest::Manifest::Build::Warnings;

    const std::array cases{
        std::pair{none, std::vector<std::string_view>{"-w"}},
        std::pair{default_, std::vector<std::string_view>{}},
        std::pair{all, std::vector<std::string_view>{"-Wall", "-Wextra"}},
        std::pair{pedantic, std::vector<std::string_view>{"-Wall", "-Wextra",
                                                          "-Wpedantic"}},
    };

    for (const auto &[warnings, expected] : cases) {
      Flags flags{};

      flags.set_warnings(warnings);
      require_compiler_args(flags, expected);
    }
  }

  TEST_CASE("Warnings as errors flag is emitted") {
    Flags flags{};

    flags.set_warnings_as_errors(true);
    require_compiler_args(flags, std::array{"-Werror"sv});
  }

  TEST_CASE("Extra flags appear in args") {
    Flags flags{};

    const std::vector<std::string_view> expected{
        "-march=native",
        "-fno-exceptions",
        "-fno-rtti",
    };
    auto string_flags = expected |
                        std::views::transform([](std::string_view flag) {
                          return std::string(flag);
                        }) |
                        std::ranges::to<std::vector<std::string>>();

    flags.add_extra_flags(string_flags);
    require_compiler_args(flags, expected);
  }

  TEST_CASE("Compiler flags are emitted") {
    using enum manifest::Manifest::Build::Standard;
    using enum manifest::Manifest::Build::Warnings;

    Flags flags{};

    flags.set_standard(cpp23)
        .set_warnings(pedantic)
        .set_warnings_as_errors(true)
        .add_include_directory("include");

    flags.add_extra_flags({
        "-march=native",
        "-fno-rtti",
    });

    require_compiler_args(flags,
                          std::array{"-std=c++23"sv, "-Wall"sv, "-Wextra"sv,
                                     "-Wpedantic"sv, "-Werror"sv, "-Iinclude"sv,
                                     "-march=native"sv, "-fno-rtti"sv});
  }

  TEST_CASE("Optimization flags are emitted") {
    using enum manifest::Manifest::Profile::Optimization;

    const std::array cases{
        std::pair{O0, "-O0"sv},
        std::pair{O1, "-O1"sv},
        std::pair{O2, "-O2"sv},
        std::pair{O3, "-O3"sv},
    };

    for (const auto &[level, expected] : cases) {
      Flags flags{};

      flags.set_optimization(level);
      require_compiler_args(flags, std::array{expected});
    }
  }

  TEST_CASE("Debug flags are emitted") {
    const std::array cases{
        std::pair{true, "-g"sv},
        std::pair{false, "-DNDEBUG"sv},
    };

    for (const auto &[debug, expected] : cases) {
      Flags flags{};

      flags.set_debug(debug);
      require_compiler_args(flags, std::array{expected});
    }
  }
} // namespace drum::compiler_flags::test
