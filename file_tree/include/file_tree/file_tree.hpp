#ifndef DRUM_FILE_TREE_HPP
#define DRUM_FILE_TREE_HPP

#include <cstddef>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace drum::ft {
  class Node {
    using dirs = std::vector<std::unique_ptr<Node>>;
    std::string_view name;
    std::variant<std::string, dirs> tree;
    dirs &children();
    static bool valid(std::string_view name);

  public:
    const dirs &children() const;
    const std::string &get_content() const;
    std::string_view get_name() const;
    enum class NodeType { File, Directory };
    NodeType type;
    Node(std::string_view name, std::string content);
    Node(std::string_view name);
    Node(std::string_view name, size_t n);
    Node &push_dir(std::string_view name);
    Node &push_file(std::string_view name, std::string content);
  };
} // namespace drum::ft

#endif // !DRUM_FILE_TREE_HPP
