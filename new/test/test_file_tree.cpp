module;
#include <catch2/catch_test_macros.hpp>

module new_cmd:test_file_tree;

import :file_tree;

namespace drum::new_cmd::ft::test {
  namespace {
    constexpr std::string_view name = "node_name";
  } // namespace

  TEST_CASE("File Tree dir contruction") {
    const Node dir1(name);
    REQUIRE(dir1.get_name() == "node_name");
    REQUIRE(dir1.type == Node::NodeType::Directory);
    REQUIRE(dir1.children().empty());

    const Node dir2(name, 3);
    REQUIRE(dir2.children().capacity() >= 3);
    REQUIRE(dir2.children().empty());
  }

  TEST_CASE("File Tree file contruction") {
    const Node file(name, "content");
    REQUIRE(file.get_name() == "node_name");
    REQUIRE(file.type == Node::NodeType::File);
    REQUIRE(file.get_content() == "content");
  }

  TEST_CASE("Invalid file operations") {
    const Node file1(name, "content");
    Node file2(name, "content");
    REQUIRE_THROWS_AS(file1.children(), std::bad_variant_access);
    REQUIRE_THROWS_AS(file2.push_dir("new_dir"), std::bad_variant_access);
  }

  TEST_CASE("Valid file operations") {
    const Node file(name, "content");
    REQUIRE_NOTHROW(file.get_content());
    REQUIRE(file.get_content() == "content");
  }

  TEST_CASE("Invalid directory operations") {
    const Node dir(name);
    REQUIRE_THROWS_AS(dir.get_content(), std::bad_variant_access);
  }

  TEST_CASE("Valid directory operations") {
    const Node dir1(name);
    Node dir2(name);
    REQUIRE_NOTHROW(dir1.children());
    REQUIRE_NOTHROW(dir2.push_file("new_file", "content"));
  }

  TEST_CASE("Push dir and file") {
    Node tmp_dir(name);
    tmp_dir.push_dir(name);
    tmp_dir.push_file(name, "content");
    const auto &ref = tmp_dir;

    REQUIRE_NOTHROW(ref.children());
    REQUIRE(ref.children().size() == 2);

    const auto &new_dir = *ref.children()[0];
    REQUIRE(new_dir.get_name() == name);
    REQUIRE_NOTHROW(new_dir.children());

    const auto &new_file = *ref.children()[1];
    REQUIRE(new_file.get_name() == name);
    REQUIRE_NOTHROW(new_file.get_content());
    REQUIRE(new_file.get_content() == "content");
  }
} // namespace drum::new_cmd::ft::test
