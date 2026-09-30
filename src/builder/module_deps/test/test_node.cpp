module;

#include <catch2/catch_test_macros.hpp>

module module_deps:test_node;

import std;

import :node;

import compile_unit;
import glaze;
import test_util;

using drum::glaze::p1689_types::Rule;

namespace drum::builder_cmd::module_deps::node::test {

  namespace {

    auto plain_rule = glaze::p1689_types::Rule{"plain.cpp.o"};
    auto alpha_rule = glaze::p1689_types::Rule{
        .primary_output = "alpha.cppm.o",
        .provides = std::vector{glaze::p1689_types::ProvidedModule{
            .is_interface = true, .logical_name = "alpha"}}};
    auto beta_rule = glaze::p1689_types::Rule{
        .primary_output = "beta.cppm.o",
        .provides = std::vector{glaze::p1689_types::ProvidedModule{
            .is_interface = true, .logical_name = "beta"}}};
    auto import_a_rule = glaze::p1689_types::Rule{
        .primary_output = "importer_a.cpp.o",
        .requires_modules = std::vector{
            glaze::p1689_types::ModuleDependency{.logical_name = "alpha"}}};
    auto import_a_b_rule = glaze::p1689_types::Rule{
        .primary_output = "importer_ab.cpp.o",
        .requires_modules = std::vector{
            glaze::p1689_types::ModuleDependency{.logical_name = "alpha"},
            glaze::p1689_types::ModuleDependency{.logical_name = "beta"}}};

    auto plain = compile_unit::TranslationUnit{"plain.cpp", "plain.cpp.o"};
    auto module_a = compile_unit::TranslationUnit{"alpha.cppm", "alpha.cppm.o"};
    auto module_b = compile_unit::TranslationUnit{"beta.cppm", "beta.cppm.o"};
    auto import_a =
        compile_unit::TranslationUnit{"importer_a.cpp", "importer_a.cpp.o"};
    auto import_a_b =
        compile_unit::TranslationUnit{"importer_ab.cpp", "importer_ab.cpp.o"};

    auto plain_node = Node{plain};
    auto alpha_node = Node{module_a, Module{"alpha", "alpha.cppm.pcm"}};
    auto beta_node = Node{module_b, Module{"beta", "beta.cppm.pcm"}};
    auto import_a_node = Node{import_a, std::nullopt, {"alpha"}};
    auto import_a_b_node = Node{import_a_b, std::nullopt, {"alpha", "beta"}};

    auto
    make_provide_map(std::vector<std::pair<std::string_view, Node *>> pairs)
        -> std::unordered_map<std::string_view, Node *> {
      std::unordered_map<std::string_view, Node *> provide_map{};

      for (auto pair : pairs) {
        provide_map.emplace(pair.first, pair.second);
      }

      return provide_map;
    }

  } // namespace

  TEST_CASE("Returns empty node list for empty input") {
    const std::vector<compile_unit::TranslationUnit> units{};
    const std::vector<Rule> rules{};

    const auto result = build_nodes(units, rules);

    REQUIRE(result);
    REQUIRE(result->empty());
  }

  TEST_CASE("Creates nodes for mixed unit types") {
    test_util::TestEnvironment env{};

    test_util::write_file(plain.source, "");
    test_util::write_file(module_a.source, "");
    test_util::write_file(import_a.source, "");

    const std::vector<compile_unit::TranslationUnit> units{plain, module_a,
                                                           import_a};
    const std::vector<Rule> rules{plain_rule, alpha_rule, import_a_rule};

    const auto result = build_nodes(units, rules);

    REQUIRE(result);
    REQUIRE(result->size() == 3);

    const auto &plain_unit = (*result)[0];
    REQUIRE(plain_unit.unit.source == plain.source);
    REQUIRE(plain_unit.unit.object == plain.object);
    REQUIRE_FALSE(plain_unit.module);
    REQUIRE(plain_unit.imports.empty());

    const auto &alpha_unit = (*result)[1];
    REQUIRE(alpha_unit.unit.source == module_a.source);
    REQUIRE(alpha_unit.unit.object == module_a.object);
    REQUIRE(alpha_unit.module);
    REQUIRE(alpha_unit.module->name == "alpha");
    REQUIRE(alpha_unit.module->bmi == "alpha.cppm.pcm");
    REQUIRE(alpha_unit.imports.empty());

    const auto &import_unit = (*result)[2];
    REQUIRE(import_unit.unit.source == import_a.source);
    REQUIRE(import_unit.unit.object == import_a.object);
    REQUIRE_FALSE(import_unit.module);
    REQUIRE(import_unit.imports == std::vector<std::string>{"alpha"});
  }

  TEST_CASE("Errors when unit and rule counts differ") {
    test_util::TestEnvironment env{};

    const std::vector<compile_unit::TranslationUnit> units{plain, module_a};
    const std::vector<Rule> rules{plain_rule};

    const auto result = build_nodes(units, rules);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Translation unit count (2) does not match "
                              "dependency rule count (1)");
  }

  TEST_CASE("Errors when no unit maps to the rule output") {
    test_util::TestEnvironment env{};

    test_util::write_file(plain.source, "");

    const glaze::p1689_types::Rule missing_rule{"missing.cpp.o"};

    const std::vector<compile_unit::TranslationUnit> units{plain};
    const std::vector<Rule> rules{missing_rule};

    const auto result = build_nodes(units, rules);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "No source mapped to output: missing.cpp.o");
  }

  TEST_CASE("Errors when the source file is not accessible") {
    test_util::TestEnvironment env{};

    const std::vector<compile_unit::TranslationUnit> units{plain};
    const std::vector<Rule> rules{plain_rule};

    const auto result = build_nodes(units, rules);

    REQUIRE_FALSE(result);
    REQUIRE(
        result.error().starts_with("Source file not accessible: plain.cpp"));
  }

  TEST_CASE("Leaves module unset when provides is empty") {
    test_util::TestEnvironment env{};

    test_util::write_file(module_a.source, "");

    const Rule empty_provides_rule{
        .primary_output = "alpha.cppm.o",
        .provides = std::vector<glaze::p1689_types::ProvidedModule>{}};

    const std::vector<compile_unit::TranslationUnit> units{module_a};
    const std::vector<Rule> rules{empty_provides_rule};

    const auto result = build_nodes(units, rules);

    REQUIRE(result);
    REQUIRE(result->size() == 1);
    REQUIRE_FALSE((*result)[0].module);
  }

  TEST_CASE("Returns empty provide map for empty input") {
    std::vector<Node> nodes{};

    auto result = build_provide_map(nodes);

    REQUIRE(result);
    REQUIRE(result->empty());
  }

  TEST_CASE("Maps each module-bearing node to its logical name") {
    std::vector<Node> nodes{plain_node, alpha_node, import_a_node, beta_node};

    auto result = build_provide_map(nodes);

    REQUIRE(result);
    REQUIRE(result->size() == 2);

    REQUIRE(result->contains(alpha_node.module->name));
    REQUIRE((*result)[alpha_node.module->name] == &nodes[1]);

    REQUIRE(result->contains(beta_node.module->name));
    REQUIRE((*result)[beta_node.module->name] == &nodes[3]);
  }

  TEST_CASE("Returns empty provide map when no nodes have a module") {
    std::vector<Node> nodes{plain_node, import_a_node, import_a_b_node};

    auto result = build_provide_map(nodes);

    REQUIRE(result);
    REQUIRE(result->empty());
  }

  TEST_CASE("Returns error on duplicate module name") {
    std::vector<Node> nodes{plain_node, alpha_node, alpha_node, import_a_node,
                            beta_node};

    auto result = build_provide_map(nodes);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Duplicate module: alpha");
  }

  TEST_CASE("Links nothing when no imports exist") {
    std::vector<Node> nodes{plain_node, alpha_node, beta_node};
    auto provide_map = make_provide_map({{alpha_node.module->name, &nodes[1]},
                                         {beta_node.module->name, &nodes[2]}});

    auto result = link_dependencies(provide_map, nodes);

    REQUIRE(result);

    for (const auto &node : nodes) {
      REQUIRE(node.dependencies.empty());
      REQUIRE(node.dependents.empty());
    }
  }

  TEST_CASE("Resolves single and multiple imports with correct back-links") {
    std::vector<Node> nodes{plain_node, alpha_node, beta_node, import_a_node,
                            import_a_b_node};
    auto provide_map = make_provide_map({{alpha_node.module->name, &nodes[1]},
                                         {beta_node.module->name, &nodes[2]}});

    auto result = link_dependencies(provide_map, nodes);

    REQUIRE(result);

    REQUIRE(nodes[1].dependents.size() == 2);
    REQUIRE(nodes[1].dependents[0] == &nodes[3]);
    REQUIRE(nodes[1].dependents[1] == &nodes[4]);

    REQUIRE(nodes[2].dependents.size() == 1);
    REQUIRE(nodes[2].dependents[0] == &nodes[4]);

    REQUIRE(nodes[3].dependencies.size() == 1);
    REQUIRE(nodes[3].dependencies[0] == &nodes[1]);

    REQUIRE(nodes[4].dependencies.size() == 2);
    REQUIRE(nodes[4].dependencies[0] == &nodes[1]);
    REQUIRE(nodes[4].dependencies[1] == &nodes[2]);
  }

  TEST_CASE("Returns error and leaves no partial links on missing module") {
    std::vector<Node> nodes{plain_node, alpha_node, import_a_b_node};
    auto provide_map = make_provide_map({{alpha_node.module->name, &nodes[1]}});

    auto result = link_dependencies(provide_map, nodes);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Module not found: beta");
  }

} // namespace drum::builder_cmd::module_deps::node::test
