module mod_deps:flatten;

import std;

import :node;

namespace drum::builder_cmd::mod_deps::flatten {
  void flatten_transitive(std::vector<node::Node *> nodes) {
    for (auto *node : nodes) {
      for (auto *dep : node->dependencies) {
        node->transitive_deps.insert(dep);
        node->transitive_deps.insert_range(dep->transitive_deps);
      }
    }
  }
} // namespace drum::builder_cmd::mod_deps::flatten
