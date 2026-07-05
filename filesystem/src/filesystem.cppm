export module filesystem;
import std;
import file_tree;

namespace drum::fs {

  export std::optional<std::string> build_file_tree(const ft::Node &file_tree);

} // namespace drum::fs
