module module_deps;

import std;

import :comp_db;
import :scan_deps;
import :node;
import :topo_sort;
import :flatten;

import compiler_flags;
import compile_unit;

namespace fs = std::filesystem;

namespace drum::builder_cmd::module_deps {

  struct NodeGraph {
    std::vector<node::Node> nodes{};
    std::vector<node::Node *> sorted{};
  };

  [[nodiscard]]
  std::expected<std::vector<compile_unit::TranslationUnit>, std::string>
  order(std::vector<compile_unit::TranslationUnit> units,
        const compiler_flags::Flags &flags, const fs::path &output_dir) {

    auto result = comp_db::generate_compile_database(units, flags, output_dir)
                      .and_then(scan_deps::scan_deps)
                      .and_then(scan_deps::parse);

    if (!result) [[unlikely]]
      return std::unexpected{std::move(result).error()};

    const auto p1689_info = std::move(*result);

    return node::build_nodes(std::move(units), std::move(p1689_info.rules))

        .and_then([](auto nodes) {
          return node::build_provide_map(nodes)
              .and_then([&](auto provide_map) {
                return node::link_dependencies(provide_map, nodes);
              })
              .transform([&]() { return std::move(nodes); });
        })

        .and_then([](auto nodes) -> std::expected<NodeGraph, std::string> {
          auto result = topo_sort::topo_sort(nodes);
          if (!result) [[unlikely]]
            return std::unexpected{std::move(result).error()};

          flatten::flatten_transitive(*result);
          return NodeGraph{std::move(nodes), std::move(*result)};
        })

        .transform([](NodeGraph node_graph) {
          for (auto &node : node_graph.nodes) {
            node.unit.dependency_bmis.reserve(node.transitive_deps.size());
            std::ranges::transform(
                node.transitive_deps,
                std::back_inserter(node.unit.dependency_bmis),
                [](node::Node *node) {
                  return compile_unit::BMI{node->module->name,
                                           node->module->bmi};
                });
          }

          std::vector<compile_unit::TranslationUnit> sorted{};
          sorted.reserve(node_graph.nodes.size());

          std::ranges::transform(
              node_graph.sorted, std::back_inserter(sorted),
              [](node::Node *node) { return std::move(node->unit); });

          return sorted;
        });
  }

} // namespace drum::builder_cmd::module_deps
