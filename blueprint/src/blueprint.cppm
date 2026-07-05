export module blueprint;
import std;
import file_tree;

namespace drum::bp {
  export ft::Node create_new_package(std::string_view pkg_name, bool lib);

} // namespace drum::bp
