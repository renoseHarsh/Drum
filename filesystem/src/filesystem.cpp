#include "file_tree/file_tree.hpp"
#include "filesystem/filesystem.hpp"
#include <filesystem>
#include <format>
#include <fstream>
#include <ios>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>

namespace drum::fs {

  constexpr std::string_view create_file_fail = "Failed to create file '{}'";
  constexpr std::string_view write_file_fail = "Failed to write to file '{}'";
  constexpr std::string_view create_dir_fail = "Failed to create dir '{}': {}";
  constexpr std::string_view dest_exists = "Destination '{}' already exists";

  std::optional<std::string> build_artifacts(const ft::Node &file_tree,
                                             std::filesystem::path path) {

    path /= file_tree.get_name();

    if (file_tree.type == ft::Node::NodeType::File) {
      std::ofstream file(path, std::ios::out | std::ios::noreplace);
      if (!file)
        return std::format(create_file_fail, file_tree.get_name());
      file << file_tree.get_content();
      file.close();
      if (!file)
        return std::format(write_file_fail, file_tree.get_name());
      return std::nullopt;
    }

    std::error_code ec;
    std::filesystem::create_directories(path, ec);

    if (ec)
      return std::format(create_dir_fail, file_tree.get_name(), ec.message());

    for (const auto &nodes : file_tree.children()) {
      if (const auto error_msg = build_artifacts(*nodes, path)) {
        return error_msg;
      }
    }

    return std::nullopt;
  }

  std::optional<std::string> build_file_tree(const ft::Node &file_tree) {

    auto root = file_tree.get_name();

    std::error_code ec;
    if (std::filesystem::exists(root, ec))
      return std::format(dest_exists, file_tree.get_name());

    return build_artifacts(file_tree, std::filesystem::current_path());
  }
} // namespace drum::fs
