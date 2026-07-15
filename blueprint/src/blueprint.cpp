module blueprint;

namespace drum::bp {
  namespace {
    constexpr std::string_view package_toml{"name = \"{}\"\n"
                                            "version = \"0.1.0\"\n"
                                            "type = \"{}\""};

    constexpr std::string_view main_cpp{"#include <iostream>\n\n"
                                        "int main() {\n"
                                        "  std::cout<<\"Hello World\\n\";\n"
                                        "}"};

  } // namespace

  ft::Node create_new_package(std::string_view pkg_name, bool lib) {

    ft::Node package(pkg_name, lib ? 3 : 2);
    package.push_file("drum.toml", std::format(package_toml, pkg_name,
                                               (lib ? "lib" : "exec")));

    auto &src_dir = package.push_dir("src");
    if (!lib) {
      src_dir.push_file("main.cpp", std::string(main_cpp));
    } else {
      auto &include = package.push_dir("include");
      include.push_dir(pkg_name);
    }

    return package;
  }

} // namespace drum::bp
