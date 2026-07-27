module new_cmd:file_tree;

import std;

namespace drum::new_cmd::ft {
  class Node {
    using dirs = std::vector<std::unique_ptr<Node>>;

  public:
    enum class NodeType { File, Directory };
    NodeType type;

    Node(std::string_view name, std::string content);
    Node(std::string_view name);
    Node(std::string_view name, std::size_t n);

    const dirs &children() const;
    std::string_view get_content() const;
    std::string_view get_name() const;
    Node &push_dir(std::string_view name);
    Node &push_file(std::string_view name, std::string content);

  private:
    std::string_view name{};
    std::variant<std::string, dirs> tree{};
    dirs &children();
  };

  Node::Node(std::string_view name, std::string content)
      : type{NodeType::File}, name{name}, tree{std::move(content)} {};

  Node::Node(std::string_view name)
      : type{NodeType::Directory}, name{name}, tree{std::in_place_type<dirs>} {}

  Node::Node(std::string_view name, std::size_t n)
      : type{NodeType::Directory}, name{name}, tree{std::in_place_type<dirs>} {
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

  std::string_view Node::get_content() const {
    return std::get<std::string>(tree);
  }

  const Node::dirs &Node::children() const {
    return std::get<Node::dirs>(tree);
  }

} // namespace drum::new_cmd::ft
