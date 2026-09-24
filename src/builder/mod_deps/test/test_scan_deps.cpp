module;

#include <catch2/catch_test_macros.hpp>

module mod_deps:test_scan_deps;

import std;

import :scan_deps;

import glaze;
import test_util;

namespace drum::builder_cmd::mod_deps::scan_deps::test {

  TEST_CASE("Runs clang-scan-deps and captures P1689 output") {
    test_util::TestEnvironment env{};

    test_util::write_file("main.cpp", "int main() {}");
    test_util::write_file(
        "compile_commands.json",
        R"([{"directory": ".", "arguments": ["clang++", "-c", "main.cpp", "-o", "main.o"], "file": "main.cpp", "output": "main.o"}])");

    const auto result = scan_deps("compile_commands.json");

    REQUIRE(result);
    REQUIRE_FALSE(result->empty());

    glaze::p1689_types::DependencyInfo info{};
    const auto ec = glaze::read_json(info, *result);

    REQUIRE_FALSE(ec);
    REQUIRE(std::ranges::any_of(info.rules,
                                [](const glaze::p1689_types::Rule &rule) {
                                  return rule.primary_output == "main.o";
                                }));
  }

  TEST_CASE("Propagates clang-scan-deps failure") {
    test_util::TestEnvironment env{};

    test_util::write_file("broken.json", "not a p1689 database");

    const auto result = scan_deps("broken.json");

    REQUIRE_FALSE(result);
    REQUIRE(result.error().empty());
  }

  TEST_CASE("Parses a minimal P1689 document") {
    const auto result = parse(
        R"({"version":0,"revision":0,"rules":[{"primary-output":"main.o"}]})");

    REQUIRE(result);
    REQUIRE(result->rules.size() == 1);
    REQUIRE(result->rules[0].primary_output == "main.o");
    REQUIRE_FALSE(result->rules[0].provides);
    REQUIRE_FALSE(result->rules[0].requires_modules);
  }

  TEST_CASE("Maps provides and requires kebab-case fields") {
    const auto result = parse(
        R"({"version":0,"revision":0,"rules":[{"primary-output":"M.cppm.o","provides":[{"is-interface":true,"logical-name":"M"}],"requires":[{"logical-name":"N"}]}]})");

    REQUIRE(result);

    const auto &rule = result->rules[0];
    REQUIRE(rule.primary_output == "M.cppm.o");

    REQUIRE(rule.provides);
    REQUIRE(rule.provides->size() == 1);
    REQUIRE((*rule.provides)[0].is_interface);
    REQUIRE((*rule.provides)[0].logical_name == "M");

    REQUIRE(rule.requires_modules);
    REQUIRE((*rule.requires_modules)[0].logical_name == "N");
  }

  TEST_CASE("Rejects malformed JSON") {
    const auto result = parse("not json");

    REQUIRE_FALSE(result);
    REQUIRE_FALSE(result.error().empty());
  }

  TEST_CASE("Rejects a field of the wrong type") {
    const auto result = parse(R"({"version":"zero","revision":0,"rules":[]})");

    REQUIRE_FALSE(result);
    REQUIRE_FALSE(result.error().empty());
  }

} // namespace drum::builder_cmd::mod_deps::scan_deps::test
