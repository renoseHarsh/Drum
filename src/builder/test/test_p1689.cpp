module;

#include <catch2/catch_test_macros.hpp>

module builder_cmd:test_p1689;

import std;

import :p1689;
import :compiler;

import test_util;
import glaze;

namespace drum::builder_cmd::p1689::test {
  TEST_CASE("Creates empty database") {
    test_util::TestEnvironment env{};

    const std::vector<SourceObject> source_objects{};
    compiler::Compiler compiler{};

    const auto result = generate(source_objects, compiler, "p1689.json");

    REQUIRE(result);

    std::vector<DatabaseEntry> database{};
    std::string buffer{};

    const auto ec = glaze::read_file_json(database, "p1689.json", buffer);

    REQUIRE_FALSE(ec);
    REQUIRE(database.empty());
  }

  TEST_CASE("Creates database entries") {
    test_util::TestEnvironment env{};

    std::vector<SourceObject> source_objects{
        {"main.cppm", "main.o"},
        {"foo.cpp", "foo.o"},
        {"bar.cppm", "bar.o"},
    };

    compiler::Compiler compiler{};
    manifest::Manifest manifest{};
    compiler.set_standard(manifest.build.standard)
        .set_warnings(manifest.build.warnings)
        .set_warnings_as_errors(true);

    const auto result = generate(source_objects, compiler, "p1689.json");

    REQUIRE(result);

    std::vector<DatabaseEntry> database;
    std::string buffer{};

    const auto ec = glaze::read_file_json(database, "p1689.json", buffer);

    REQUIRE_FALSE(ec);
    REQUIRE(database.size() == source_objects.size());

    for (const auto &[source_object, entry] :
         std::views::zip(source_objects, database)) {
      const auto &[source, object] = source_object;

      REQUIRE(entry.directory == ".");
      REQUIRE(entry.file == source);
      REQUIRE(entry.output == object);

      REQUIRE(!std::ranges::search(entry.arguments, compiler.args()).empty());

      REQUIRE(!std::ranges::search(
                   entry.arguments,
                   std::array<std::string, 4>{"-c", source.string(), "-o",
                                              object.string()})
                   .empty());
    }
  }

  TEST_CASE("Returns error when database cannot be written") {
    test_util::TestEnvironment env{};

    const std::vector<SourceObject> source_objects{
        {"main.cppm", "main.o"},
    };
    compiler::Compiler compiler{};

    const auto result =
        generate(source_objects, compiler, "nonexistent_directory/p1689.json");

    REQUIRE_FALSE(result);
    REQUIRE_FALSE(result.error().empty());
  }
} // namespace drum::builder_cmd::p1689::test
