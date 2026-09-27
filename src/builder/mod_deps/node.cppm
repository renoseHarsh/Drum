module mod_deps:node;

import std;

import :units;

namespace drum::builder_cmd::mod_deps::node {

  struct Node {
    units::TranslationUnit unit{};

    std::vector<Node *> dependencies{};
    std::vector<Node *> dependents{};
  };

  [[nodiscard]]
  std::vector<Node> build_nodes(std::vector<units::TranslationUnit> units) {
    std::vector<Node> nodes{};
    nodes.reserve(units.size());

    std::ranges::transform(
        units, std::back_inserter(nodes),
        [](units::TranslationUnit &unit) { return Node{std::move(unit)}; });

    return nodes;
  }

  [[nodiscard]]
  std::expected<std::unordered_map<std::string_view, Node *>, std::string>
  build_provide_map(std::vector<Node> &nodes) {
    std::unordered_map<std::string_view, Node *> provide_map{};

    auto modules_only = nodes | std::views::filter([](const auto &node) {
                          return node.unit.module.has_value();
                        });

    for (auto &node : modules_only) {
      std::string_view module_name{node.unit.module->name};
      if (provide_map.contains(module_name))
        return std::unexpected{
            std::format("Duplicate module: {}", module_name)};

      provide_map.emplace(module_name, &node);
    }

    return provide_map;
  }

  [[nodiscard]]
  std::expected<std::vector<Node>, std::string>
  link_dependencies(std::unordered_map<std::string_view, Node *> provide_map,
                    std::vector<Node> &nodes) {
    for (auto &node : nodes) {
      node.dependencies.reserve(node.unit.imports.size());
      for (auto &dependency : node.unit.imports) {
        auto module = provide_map.find(dependency);
        if (module == provide_map.end()) [[unlikely]]
          return std::unexpected{
              std::format("Module not found: {}", dependency)};

        node.dependencies.push_back(module->second);
        module->second->dependents.push_back(&node);
      }
    }

    return std::move(nodes);
  }

} // namespace drum::builder_cmd::mod_deps::node
