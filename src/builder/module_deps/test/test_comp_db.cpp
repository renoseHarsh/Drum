module;

#include <catch2/catch_test_macros.hpp>

module module_deps:test_comp_db;

import std;

import :comp_db;

import test_util;
import compiler_flags;
import compile_unit;
import glaze;

namespace drum::builder_cmd::module_deps::comp_db::test {

  TEST_CASE("Creates empty database") {
    test_util::TestEnvironment env{};

    const std::vector<compile_unit::TranslationUnit> units{};
    compiler_flags::Flags flags{};

    const auto result = generate_compile_database(units, flags, ".");

    REQUIRE(result);

    const fs::path &p1689_file = *result;
    std::vector<DatabaseEntry> database{};
    std::string buffer{};

    const auto ec =
        glaze::read_file_json(database, p1689_file.string(), buffer);

    REQUIRE_FALSE(ec);
    REQUIRE(database.empty());
  }

  TEST_CASE("Creates database entries") {
    test_util::TestEnvironment env{};

    std::vector<compile_unit::TranslationUnit> units{
        {"main.cppm", "main.cppm.o"},
        {"foo.cpp", "foo.cpp.o"},
        {"bar.cppm", "bar.cppm.o"},
    };

    compiler_flags::Flags flags{};
    manifest::Manifest manifest{};
    flags.set_standard(manifest.build.standard)
        .set_warnings(manifest.build.warnings)
        .set_warnings_as_errors(true);

    const auto result = generate_compile_database(units, flags, ".");

    REQUIRE(result);

    std::vector<DatabaseEntry> database;
    std::string buffer{};

    const auto ec = glaze::read_file_json(
        database, "scan_deps_compile_commands.json", buffer);

    REQUIRE_FALSE(ec);
    REQUIRE(database.size() == units.size());

    for (const auto &[unit, entry] : std::views::zip(units, database)) {

      REQUIRE(entry.directory == ".");
      REQUIRE(entry.file == unit.source);
      REQUIRE(entry.output == unit.object);

      REQUIRE(!std::ranges::search(entry.arguments, flags.args()).empty());

      REQUIRE(!std::ranges::search(
                   entry.arguments,
                   std::array<std::string, 4>{"-c", unit.source.string(), "-o",
                                              unit.object.string()})
                   .empty());
    }
  }

  TEST_CASE("Returns error when database cannot be written") {
    test_util::TestEnvironment env{};

    const std::vector<compile_unit::TranslationUnit> units{
        {"main.cppm", "main.o"},
    };
    compiler_flags::Flags flags{};

    const auto result =
        generate_compile_database(units, flags, "nonexistent_directory");

    REQUIRE_FALSE(result);
    REQUIRE_FALSE(result.error().empty());
  }

} // namespace drum::builder_cmd::module_deps::comp_db::test
