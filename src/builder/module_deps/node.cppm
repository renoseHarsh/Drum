module module_deps:node;

import std;

import compile_unit;
import glaze;

namespace fs = std::filesystem;

namespace drum::builder_cmd::module_deps::node {

  struct Module {
    std::string name{};
    fs::path bmi{};
  };

  struct Node {
    compile_unit::TranslationUnit unit{};

    std::optional<Module> module{};
    std::vector<std::string> imports{};

    std::vector<Node *> dependencies{};
    std::vector<Node *> dependents{};
    std::set<Node *> transitive_deps{};
  };

  namespace {

    std::expected<compile_unit::TranslationUnit, std::string>
    get_unit_from_object(const fs::path &object,
                         std::vector<compile_unit::TranslationUnit> &units) {

      auto unit = std::ranges::find_if(
          units, [&](auto &unit) { return unit.object == object; });

      if (unit == units.end()) [[unlikely]]
        return std::unexpected{
            std::format("No source mapped to output: {}", object.string())};

      std::error_code ec{};
      if (!fs::exists(unit->source, ec)) [[unlikely]] {
        return std::unexpected{std::format(
            "Source file not accessible: {} ({})", unit->source.string(),
            ec ? ec.message() : "does not exist")};
      }

      return std::move(*unit);
    }

    Node make_node(glaze::p1689_types::Rule &rule,
                   compile_unit::TranslationUnit unit) {
      Node node{std::move(unit)};

      if (rule.provides && !rule.provides->empty()) {
        fs::path bmi{node.unit.object};
        bmi.replace_extension(".pcm");

        node.module =
            Module{rule.provides->front().logical_name, std::move(bmi)};
      }

      if (rule.requires_modules) {
        node.imports.reserve(rule.requires_modules->size());

        std::ranges::transform(
            *rule.requires_modules, std::back_inserter(node.imports),
            [](auto &dep) { return std::move(dep.logical_name); });
      }

      return node;
    }

  }; // namespace

  [[nodiscard]]
  std::expected<std::vector<Node>, std::string>
  build_nodes(std::vector<compile_unit::TranslationUnit> units,
              std::vector<glaze::p1689_types::Rule> rules) {
    if (units.size() != rules.size()) [[unlikely]]
      return std::unexpected{std::format("Translation unit count ({}) does not "
                                         "match dependency rule count ({})",
                                         units.size(), rules.size())};

    std::vector<Node> nodes{};
    nodes.reserve(units.size());

    for (auto &rule : rules) {
      auto result =
          get_unit_from_object(rule.primary_output, units)
              .transform([&](auto unit) { return make_node(rule, unit); });

      if (!result) [[unlikely]]
        return std::unexpected{std::move(result).error()};

      nodes.push_back(std::move(*result));
    }

    return nodes;
  }

  [[nodiscard]]
  std::expected<std::unordered_map<std::string_view, Node *>, std::string>
  build_provide_map(std::vector<Node> &nodes) {
    std::unordered_map<std::string_view, Node *> provide_map{};

    auto modules_only = nodes | std::views::filter([](const auto &node) {
                          return node.module.has_value();
                        });

    for (auto &node : modules_only) {
      std::string_view module_name{node.module->name};
      if (provide_map.contains(module_name)) [[unlikely]]
        return std::unexpected{
            std::format("Duplicate module: {}", module_name)};

      provide_map.emplace(module_name, &node);
    }

    return provide_map;
  }

  [[nodiscard]]
  std::expected<void, std::string>
  link_dependencies(std::unordered_map<std::string_view, Node *> provide_map,
                    std::vector<Node> &nodes) {
    for (auto &node : nodes) {
      node.dependencies.reserve(node.imports.size());
      for (auto &dependency : node.imports) {
        auto module = provide_map.find(dependency);
        if (module == provide_map.end()) [[unlikely]]
          return std::unexpected{
              std::format("Module not found: {}", dependency)};

        node.dependencies.push_back(module->second);
        module->second->dependents.push_back(&node);
      }
    }

    return {};
  }

} // namespace drum::builder_cmd::module_deps::node
