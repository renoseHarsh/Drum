#include "blueprint/blueprint.hpp"
#include "file_tree/file_tree.hpp"
#include <catch2/catch_test_macros.hpp>
#include <format>
#include <string_view>

namespace drum::bp::test {
  namespace {
    constexpr std::string_view package_toml = "Package Name: \"{}\"\n"
                                              "Type: \"{}\"";

    constexpr std::string_view main_cpp = "#include <iostream>\n\n"
                                          "int main() {\n"
                                          "  std::cout<<\"Hello World\\n\";\n"
                                          "}";
    constexpr std::string_view exec_pkg_name = "exec_pkg";
    constexpr std::string_view lib_pkg_name = "lib_pkg";

  } // namespace

  using NodeType = ft::Node::NodeType;

  TEST_CASE("Construct exec file tree") {
    const auto exec_tree = create_new_package(exec_pkg_name, false);

    REQUIRE(exec_tree.get_name() == exec_pkg_name);

    const auto &children = exec_tree.children();
    REQUIRE(children.size() == 2);

    const auto &toml = *children[0];
    const auto &src = *children[1];

    REQUIRE(toml.get_name() == "drum.toml");
    REQUIRE(toml.get_content() ==
            std::format(package_toml, exec_pkg_name, "exec"));

    REQUIRE(src.get_name() == "src");
    REQUIRE(src.children()[0]->get_content() == main_cpp);
  }

  TEST_CASE("Construct lib file tree") {
    const auto lib_tree = create_new_package(lib_pkg_name, true);

    REQUIRE(lib_tree.get_name() == lib_pkg_name);

    const auto &children = lib_tree.children();
    REQUIRE(children.size() == 3);

    const auto &toml = *children[0];
    const auto &src = *children[1];
    const auto &include = *children[2];

    REQUIRE(toml.get_name() == "drum.toml");
    REQUIRE(toml.get_content() ==
            std::format(package_toml, lib_pkg_name, "lib"));

    REQUIRE(src.get_name() == "src");
    REQUIRE(src.children().size() == 0);

    REQUIRE(include.get_name() == "include");
    REQUIRE(include.children().size() == 1);

    const auto &include_lib = *include.children()[0];
    REQUIRE(include_lib.get_name() == lib_pkg_name);
    REQUIRE(include_lib.children().size() == 0);
  }

} // namespace drum::bp::test
