export module blueprint;
import std;
import file_tree;

export namespace drum::bp {
  ft::Node create_new_package(std::string_view pkg_name, bool lib);
} // namespace drum::bp
