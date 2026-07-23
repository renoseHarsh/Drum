module new_cmd;

import :blueprint;
import :filesystem;

import std;

namespace drum::new_cmd {

  std::expected<void, std::string> execute(const NewArgs &args) {

    ft::Node package = args.pkg_type == NewArgs::PackageType::executable
                           ? bp::create_exec_package(args.pkg_name)
                           : bp::create_lib_package(args.pkg_name);

    return fs::build_file_tree(package);
  }
} // namespace drum::new_cmd
