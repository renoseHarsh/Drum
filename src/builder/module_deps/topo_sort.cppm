module module_deps:topo_sort;

import std;

import :node;

namespace drum::builder_cmd::module_deps::topo_sort {

  namespace {
    enum class State { UNVISITED, VISITING, VISITED };

    [[nodiscard]]
    std::expected<void, std::string>
    sort(node::Node *node,
         std::unordered_map<const node::Node *, State> &node_state,
         std::vector<node::Node *> &stack) {
      node_state[node] = State::VISITING;

      for (auto *dependent : node->dependents) {
        switch (node_state[dependent]) {
        case State::VISITING:
          return std::unexpected{
              std::format("Cycle detected: {}", node->unit.source.string())};

        case State::UNVISITED:
          if (auto result = sort(dependent, node_state, stack); !result)
            return std::unexpected{std::move(result.error())};

        default:
          break;
        }
      }

      node_state[node] = State::VISITED;
      stack.push_back(node);

      return {};
    }

  } // namespace

  [[nodiscard]]
  std::expected<std::vector<node::Node *>, std::string>
  topo_sort(std::vector<node::Node> &nodes) {
    std::unordered_map<const node::Node *, State> node_state{};
    for (const auto &node : nodes) {
      node_state[&node] = State::UNVISITED;
    }

    std::vector<node::Node *> stack{};
    stack.reserve(nodes.size());

    for (auto &node : nodes) {
      if (node_state[&node] == State::UNVISITED) {
        if (auto result = sort(&node, node_state, stack); !result)
          return std::unexpected{std::move(result.error())};
      }
    }

    std::ranges::reverse(stack);
    return stack;
  };
} // namespace drum::builder_cmd::module_deps::topo_sort
