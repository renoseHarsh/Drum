module;

#include <catch2/catch_test_macros.hpp>

module mod_deps:test_units;

import std;

import :units;

import test_util;
import glaze;

namespace drum::builder_cmd::mod_deps::units::test {

  TEST_CASE("Maps a plain source") {
    test_util::TestEnvironment env{};
    test_util::write_file("main.cpp", "int main() {}");

    const std::vector<SourceObject> source_objects{{"main.cpp", "main.cpp.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{.primary_output = "main.cpp.o"}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE(result);
    REQUIRE(result->size() == 1);

    const auto &unit = (*result)[0];
    REQUIRE(unit.source == "main.cpp");
    REQUIRE(unit.object == "main.cpp.o");
    REQUIRE_FALSE(unit.module);
    REQUIRE(unit.imports.empty());
  }

  TEST_CASE("Maps a module interface unit") {
    test_util::TestEnvironment env{};
    test_util::write_file("M.cppm", "export module M;");

    const std::vector<SourceObject> source_objects{{"M.cppm", "M.cppm.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{
                .primary_output = "M.cppm.o",
                .provides = std::vector{glaze::p1689_types::ProvidedModule{
                    .is_interface = true, .logical_name = "M"}}}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE(result);
    REQUIRE(result->size() == 1);

    const auto &unit = (*result)[0];
    REQUIRE(unit.source == "M.cppm");
    REQUIRE(unit.object == "M.cppm.o");
    REQUIRE(unit.module);
    REQUIRE(unit.module->name == "M");
    REQUIRE(unit.module->bmi == "M.cppm.pcm");
    REQUIRE(unit.imports.empty());
  }

  TEST_CASE("Records imported modules") {
    test_util::TestEnvironment env{};
    test_util::write_file("user.cpp", "import M;");

    const std::vector<SourceObject> source_objects{{"user.cpp", "user.cpp.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{
                .primary_output = "user.cpp.o",
                .requires_modules = std::vector{
                    glaze::p1689_types::ModuleDependency{.logical_name = "M"},
                    glaze::p1689_types::ModuleDependency{.logical_name = "N"}}}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE(result);
    REQUIRE(result->size() == 1);

    const auto &unit = (*result)[0];
    REQUIRE(unit.source == "user.cpp");
    REQUIRE(unit.object == "user.cpp.o");
    REQUIRE_FALSE(unit.module);
    REQUIRE(unit.imports == std::vector<std::string>{"M", "N"});
  }

  TEST_CASE("Errors when no source maps to the output") {
    test_util::TestEnvironment env{};

    const std::vector<SourceObject> source_objects{{"main.cpp", "main.cpp.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{.primary_output = "missing.cpp.o"}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "No source mapped to output: missing.cpp.o");
  }

  TEST_CASE("Errors when the source file is not on disk") {
    test_util::TestEnvironment env{};

    const std::vector<SourceObject> source_objects{{"gone.cpp", "gone.cpp.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{.primary_output = "gone.cpp.o"}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE_FALSE(result);
    REQUIRE(result.error().starts_with("Source file not accessible: gone.cpp"));
  }

  TEST_CASE("Empty provides leaves module unset") {
    test_util::TestEnvironment env{};
    test_util::write_file("M.cppm", "export module M;");

    const std::vector<SourceObject> source_objects{{"M.cppm", "M.cppm.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{
                .primary_output = "M.cppm.o",
                .provides = std::vector<glaze::p1689_types::ProvidedModule>{}}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE(result);
    REQUIRE(result->size() == 1);
    REQUIRE_FALSE((*result)[0].module);
  }

  TEST_CASE("Collects every translation unit in order") {
    test_util::TestEnvironment env{};
    test_util::write_file("main.cpp", "int main() {}");
    test_util::write_file("M.cppm", "export module M;");
    test_util::write_file("user.cpp", "import M;");

    const std::vector<SourceObject> source_objects{
        {"main.cpp", "main.cpp.o"},
        {"M.cppm", "M.cppm.o"},
        {"user.cpp", "user.cpp.o"},
    };

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{.primary_output = "main.cpp.o"},
            glaze::p1689_types::Rule{
                .primary_output = "M.cppm.o",
                .provides = std::vector{glaze::p1689_types::ProvidedModule{
                    .is_interface = true, .logical_name = "M"}}},
            glaze::p1689_types::Rule{
                .primary_output = "user.cpp.o",
                .requires_modules = std::vector{
                    glaze::p1689_types::ModuleDependency{.logical_name = "M"}}}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE(result);
    REQUIRE(result->size() == 3);

    const auto &main_unit = (*result)[0];
    REQUIRE(main_unit.source == "main.cpp");
    REQUIRE_FALSE(main_unit.module);
    REQUIRE(main_unit.imports.empty());

    const auto &module_unit = (*result)[1];
    REQUIRE(module_unit.source == "M.cppm");
    REQUIRE(module_unit.module);
    REQUIRE(module_unit.module->name == "M");
    REQUIRE(module_unit.module->bmi == "M.cppm.pcm");

    const auto &user_unit = (*result)[2];
    REQUIRE(user_unit.source == "user.cpp");
    REQUIRE(user_unit.imports == std::vector<std::string>{"M"});
  }

  TEST_CASE("Fails fast on the first invalid rule") {
    test_util::TestEnvironment env{};
    test_util::write_file("main.cpp", "int main() {}");

    const std::vector<SourceObject> source_objects{{"main.cpp", "main.cpp.o"}};

    const glaze::p1689_types::DependencyInfo info{
        .revision = 0,
        .rules = std::vector{
            glaze::p1689_types::Rule{.primary_output = "main.cpp.o"},
            glaze::p1689_types::Rule{.primary_output = "missing.cpp.o"}},
        .version = 0,
    };

    const auto result = collect_translation_units(info, source_objects);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "No source mapped to output: missing.cpp.o");
  }

} // namespace drum::builder_cmd::mod_deps::units::test