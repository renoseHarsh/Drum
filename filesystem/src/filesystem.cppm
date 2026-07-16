export module filesystem;
import std;
import file_tree;

export namespace drum::fs {
  std::optional<std::string> build_file_tree(const ft::Node &file_tree);
} // namespace drum::fs
