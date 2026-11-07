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

  TEST_CASE("Extra flags are added") {
    std::vector<std::string> extra_args = {"-O1", "-g"};
    std::array cmp_args{"-O1"sv, "-g"sv};

    Flags flags{};

    flags.add_flags(extra_args);
    require_compiler_args(flags, cmp_args);
  }

} // namespace drum::compiler_flags::test
