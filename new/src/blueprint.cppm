module new_cmd:blueprint;

import std;

import :file_tree;

namespace drum::new_cmd::bp {
  namespace {
    constexpr std::string_view package_toml{"name = \"{}\"\n"
                                            "version = \"0.1.0\"\n"
                                            "type = \"{}\""};

    constexpr std::string_view main_cpp{"#include <iostream>\n\n"
                                        "int main() {\n"
                                        "  std::cout<<\"Hello World\\n\";\n"
                                        "}"};

    ft::Node &populate_common(ft::Node &tree, std::string content) {
      tree.push_file("drum.toml", std::move(content));
      return tree.push_dir("src");
    }

  } // namespace

  ft::Node create_exec_package(std::string_view pkg_name) {
    ft::Node package{pkg_name, 2};

    auto &src =
        populate_common(package, std::format(package_toml, pkg_name, "exec"));

    src.push_file("main.cpp", std::string{main_cpp});

    return package;
  }
  ft::Node create_lib_package(std::string_view pkg_name) {
    ft::Node package{pkg_name, 3};

    populate_common(package, std::format(package_toml, pkg_name, "lib"));

    auto &include = package.push_dir("include");
    include.push_dir(pkg_name);

    return package;
  }

} // namespace drum::new_cmd::bp
