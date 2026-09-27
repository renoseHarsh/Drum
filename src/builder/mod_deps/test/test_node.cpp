module;

#include <catch2/catch_test_macros.hpp>

module mod_deps:test_node;

import std;

import :node;
import :units;

namespace drum::builder_cmd::mod_deps::node::test {

  namespace {

    auto plain_source = units::TranslationUnit{"plain.cpp", "plain.cpp.o"};
    auto module_a = units::TranslationUnit{
        "alpha.cppm", "alpha.cppm.o",
        units::TranslationUnit::Module{"alpha", "alpha.cppm.o"}};
    auto module_b = units::TranslationUnit{
        "beta.cppm", "beta.cppm.o",
        units::TranslationUnit::Module{"beta", "beta.cppm.o"}};
    auto import_a = units::TranslationUnit{
        "importer_a.cpp", "importer_a.cpp.o", {}, {"alpha"}};
    auto import_a_b = units::TranslationUnit{
        "importer_ab.cpp", "importer_ab.cpp.o", {}, {"alpha", "beta"}};

    auto plain_node = Node{plain_source};
    auto alpha_node = Node{module_a};
    auto beta_node = Node{module_b};
    auto import_a_node = Node{import_a};
    auto import_a_b_node = Node{import_a_b};

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
    std::vector<units::TranslationUnit> units{};
    auto nodes = build_nodes(units);
    REQUIRE(nodes.empty());
  }

  TEST_CASE("Creates valid nodes for mixed unit shapes") {
    std::vector<units::TranslationUnit> units{plain_source, module_a, import_a};
    auto nodes = build_nodes(units);

    REQUIRE(nodes.size() == 3);

    REQUIRE(nodes[0].unit.source == plain_source.source);
    REQUIRE_FALSE(nodes[0].unit.module.has_value());
    REQUIRE(nodes[0].unit.imports.empty());
    REQUIRE(nodes[0].dependencies.empty());
    REQUIRE(nodes[0].dependents.empty());

    REQUIRE(nodes[1].unit.source == module_a.source);
    REQUIRE(nodes[1].unit.module.has_value());
    REQUIRE(nodes[1].unit.module->name == module_a.module->name);
    REQUIRE(nodes[1].unit.module->bmi == module_a.module->bmi);
    REQUIRE(nodes[1].dependencies.empty());
    REQUIRE(nodes[1].dependents.empty());

    REQUIRE(nodes[2].unit.source == import_a.source);
    REQUIRE(nodes[2].unit.imports == import_a.imports);
    REQUIRE(nodes[2].dependencies.empty());
    REQUIRE(nodes[2].dependents.empty());
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

    REQUIRE(result->contains(module_a.module->name));
    REQUIRE((*result)[module_a.module->name] == &nodes[1]);

    REQUIRE(result->contains(module_b.module->name));
    REQUIRE((*result)[module_b.module->name] == &nodes[3]);
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
    auto provide_map = make_provide_map({{module_a.module->name, &nodes[1]},
                                         {module_b.module->name, &nodes[2]}});

    auto result = link_dependencies(provide_map, nodes);

    REQUIRE(result);
    auto &linked_nodes = *result;

    for (const auto &node : linked_nodes) {
      REQUIRE(node.dependencies.empty());
      REQUIRE(node.dependents.empty());
    }
  }

  TEST_CASE("Resolves single and multiple imports with correct back-links") {
    std::vector<Node> nodes{plain_node, alpha_node, beta_node, import_a_node,
                            import_a_b_node};
    auto provide_map = make_provide_map({{module_a.module->name, &nodes[1]},
                                         {module_b.module->name, &nodes[2]}});

    auto result = link_dependencies(provide_map, nodes);

    REQUIRE(result);
    auto &linked_nodes = *result;

    REQUIRE(linked_nodes[1].dependents.size() == 2);
    REQUIRE(linked_nodes[1].dependents[0] == &linked_nodes[3]);
    REQUIRE(linked_nodes[1].dependents[1] == &linked_nodes[4]);

    REQUIRE(linked_nodes[2].dependents.size() == 1);
    REQUIRE(linked_nodes[2].dependents[0] == &linked_nodes[4]);

    REQUIRE(linked_nodes[3].dependencies.size() == 1);
    REQUIRE(linked_nodes[3].dependencies[0] == &linked_nodes[1]);

    REQUIRE(linked_nodes[4].dependencies.size() == 2);
    REQUIRE(linked_nodes[4].dependencies[0] == &linked_nodes[1]);
    REQUIRE(linked_nodes[4].dependencies[1] == &linked_nodes[2]);
  }

  TEST_CASE("Returns error and leaves no partial links on missing module") {
    std::vector<Node> nodes{plain_node, alpha_node, import_a_b_node};
    auto provide_map = make_provide_map({{module_a.module->name, &nodes[1]}});

    auto result = link_dependencies(provide_map, nodes);

    REQUIRE_FALSE(result);
    REQUIRE(result.error() == "Module not found: beta");
  }

} // namespace drum::builder_cmd::mod_deps::node::test
