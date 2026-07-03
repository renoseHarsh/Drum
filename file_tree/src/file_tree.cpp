#include "file_tree/file_tree.hpp"
#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

namespace drum::ft {
  Node::Node(std::string_view name, std::string content)
      : name{name}, tree{std::move(content)}, type{NodeType::File} {};

  Node::Node(std::string_view name)
      : name{name}, tree{std::in_place_type<dirs>}, type{NodeType::Directory} {}

  Node::Node(std::string_view name, std::size_t n)
      : name{name}, tree{std::in_place_type<dirs>}, type{NodeType::Directory} {
    std::get<dirs>(tree).reserve(n);
  }
  Node::dirs &Node::children() { return std::get<Node::dirs>(tree); }

  Node &Node::push_dir(std::string_view name) {
    auto &c = children();
    c.push_back(std::make_unique<Node>(name));
    return *c.back();
  }

  Node &Node::push_file(std::string_view name, std::string content) {
    auto &c = children();
    c.push_back(std::make_unique<Node>(name, std::move(content)));
    return *c.back();
  }

  std::string_view Node::get_name() const { return name; }

  const std::string &Node::get_content() const {
    return std::get<std::string>(tree);
  }

  const Node::dirs &Node::children() const {
    return std::get<Node::dirs>(tree);
  }

} // namespace drum::ft
