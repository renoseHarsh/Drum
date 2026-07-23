module;
#include <catch2/catch_test_macros.hpp>
#include <catch2/reporters/catch_reporter_event_listener.hpp>
#include <catch2/reporters/catch_reporter_registrars.hpp>
#include <filesystem>

module new_cmd:test_filesystem;

import :filesystem;

namespace drum::new_cmd::fs::test {
  namespace {
    class TestEnvironment : public Catch::EventListenerBase {
    public:
      using Catch::EventListenerBase::EventListenerBase;
      void testRunStarting(Catch::TestRunInfo const &) override {
        temp_dir = std::filesystem::temp_directory_path() / "drum_test";
        std::filesystem::remove_all(temp_dir);
        std::filesystem::create_directory(temp_dir);

        old_cwd = std::filesystem::current_path();
        std::filesystem::current_path(temp_dir);
      }

      void testRunEnded(Catch::TestRunStats const &) override {
        std::filesystem::current_path(old_cwd);
        std::filesystem::remove_all(temp_dir);
      }

    private:
      using path = std::filesystem::path;
      path temp_dir;
      path old_cwd;
    };
    CATCH_REGISTER_LISTENER(TestEnvironment);

    std::string read_file(const std::filesystem::path &path) {
      std::ifstream file(path, std::ios::binary);
      REQUIRE(file);

      std::string content(std::filesystem::file_size(path), '\0');
      file.read(content.data(), static_cast<std::streamsize>(content.size()));
      REQUIRE(file);

      return content;
    }

    void check_valid_file(const std::filesystem::path &file_name,
                          std::string_view content) {
      REQUIRE(std::filesystem::exists(file_name));
      REQUIRE(std::filesystem::is_regular_file(file_name));

      REQUIRE(read_file(file_name) == content);
    }

    void check_valid_dir(const std::filesystem::path &path) {
      REQUIRE(std::filesystem::exists(path));
      REQUIRE(std::filesystem::is_directory(path));
    }
  } // namespace

  TEST_CASE("Create a single file") {
    constexpr std::string_view file_name = "single_file.txt";
    constexpr std::string_view file_content = "single file content";
    ft::Node file_tree(file_name, std::string(file_content));
    auto result = fs::build_file_tree(file_tree);

    REQUIRE(result.has_value() == true);
    check_valid_file(file_name, file_content);
  }

  // TEST_CASE("Empty Directory") {
  //   constexpr std::string_view dir_name = "empty_directory";
  //   ft::Node file_tree(dir_name);
  //   auto result = fs::build_file_tree(file_tree);
  //
  //   REQUIRE_FALSE(result.has_value());
  //   check_valid_dir(dir_name);
  // }
  //
  // TEST_CASE("Directory with file") {
  //   constexpr std::string_view dir_name = "dir_with_one_file";
  //   constexpr std::string_view file_name = "single_dir_nested_file.txt";
  //   constexpr std::string_view file_content = "single dir\nnested file
  //   content";
  //
  //   ft::Node file_tree(dir_name);
  //   file_tree.push_file(file_name, std::string(file_content));
  //   auto result = fs::build_file_tree(file_tree);
  //
  //   auto root = std::filesystem::path(dir_name);
  //
  //   REQUIRE_FALSE(result.has_value());
  //   check_valid_dir(root);
  //
  //   check_valid_file(root / file_name, file_content);
  // }
  //
  // TEST_CASE("Nested directories and files") {
  //   constexpr std::string_view root_dir_name = "root_dir";
  //
  //   constexpr std::string_view lvl_1_dir_name = "lvl_1_dir";
  //   constexpr std::string_view lvl_1_file_name = "lvl_1_file.txt";
  //   constexpr std::string_view lvl_1_file_content = "lvl 1\nfile content";
  //
  //   constexpr std::string_view lvl_2_dir_name = "lvl_2_dir";
  //   constexpr std::string_view lvl_2_file_name = "lvl_2_file.txt";
  //   constexpr std::string_view lvl_2_file_content = "lvl 2\nfile content";
  //
  //   constexpr std::string_view lvl_3_dir_name = "lvl_3_dir";
  //   constexpr std::string_view lvl_3_file_name = "lvl_3_file.txt";
  //   constexpr std::string_view lvl_3_file_content = "lvl 3\nfile content";
  //
  //   ft::Node file_tree(root_dir_name);
  //   auto &lvl_1_dir = file_tree.push_dir(lvl_1_dir_name);
  //   file_tree.push_file(lvl_1_file_name, std::string(lvl_1_file_content));
  //
  //   auto &lvl_2_dir = lvl_1_dir.push_dir(lvl_2_dir_name);
  //   lvl_1_dir.push_file(lvl_2_file_name, std::string(lvl_2_file_content));
  //
  //   lvl_2_dir.push_dir(lvl_3_dir_name);
  //   lvl_2_dir.push_file(lvl_3_file_name, std::string(lvl_3_file_content));
  //
  //   auto result = build_file_tree(file_tree);
  //
  //   auto root = std::filesystem::path(root_dir_name);
  //
  //   REQUIRE_FALSE(result.has_value());
  //   check_valid_dir(root);
  //
  //   check_valid_file(root / lvl_1_file_name, lvl_1_file_content);
  //   root /= lvl_1_dir_name;
  //   check_valid_dir(root);
  //
  //   check_valid_file(root / lvl_2_file_name, lvl_2_file_content);
  //   root /= lvl_2_dir_name;
  //   check_valid_dir(root);
  //
  //   check_valid_file(root / lvl_3_file_name, lvl_3_file_content);
  //   root /= lvl_3_dir_name;
  //   check_valid_dir(root);
  // }
  //
  // TEST_CASE("Root already exists") {
  //   constexpr std::string_view existing = "exists";
  //   constexpr std::string_view dest_exists =
  //       "Destination 'exists' already exists";
  //
  //   std::filesystem::create_directory(existing);
  //
  //   ft::Node file_tree(existing);
  //
  //   auto result = fs::build_file_tree(file_tree);
  //   REQUIRE(result.has_value());
  //   REQUIRE(result.value() == dest_exists);
  // }

} // namespace drum::new_cmd::fs::test
