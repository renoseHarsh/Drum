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

  namespace {
    struct NodeGraph {
      std::vector<node::Node> nodes{};
      std::vector<node::Node *> sorted{};
    };

    [[nodiscard]]
    std::expected<NodeGraph, std::string> sort(std::vector<node::Node> nodes) {
      auto sorted = topo_sort::topo_sort(nodes);
      if (!sorted) [[unlikely]]
        return std::unexpected{std::move(sorted).error()};

      flatten::flatten_transitive(*sorted);
      return NodeGraph{std::move(nodes), std::move(*sorted)};
    }

    void attach_bmis(std::vector<node::Node> &nodes) {
      for (auto &node : nodes) {
        node.unit.dependency_bmis =
            node.transitive_deps |
            std::views::transform([](node::Node *dep) {
              return compile_unit::BMI{dep->module->name, dep->module->bmi};
            }) |
            std::ranges::to<std::vector>();
      }
    }

    [[nodiscard]]
    std::vector<compile_unit::TranslationUnit>
    take_units(std::vector<node::Node *> &sorted_ptr) {
      return sorted_ptr |
             std::views::transform(
                 [](auto *node_ptr) { return std::move(node_ptr->unit); }) |
             std::ranges::to<std::vector>();
    }

    [[nodiscard]]
    std::vector<compile_unit::TranslationUnit>
    to_ordered_units(NodeGraph graph) {
      attach_bmis(graph.nodes);
      return take_units(graph.sorted);
    }

  } // namespace

  [[nodiscard]]
  std::expected<std::vector<compile_unit::TranslationUnit>, std::string>
  order(std::vector<compile_unit::TranslationUnit> units,
        const compiler_flags::Flags &flags, const fs::path &output_dir) {

    auto result = comp_db::generate_compile_database(units, flags, output_dir)
                      .and_then(scan_deps::scan_deps)
                      .and_then(scan_deps::parse);

    if (!result) [[unlikely]]
      return std::unexpected{std::move(result).error()};

    auto &p1689_info = *result;

    return node::build_nodes(std::move(units), std::move(p1689_info.rules))
        .and_then(node::link)
        .and_then(sort)
        .transform(to_ordered_units);
  }

} // namespace drum::builder_cmd::module_deps
