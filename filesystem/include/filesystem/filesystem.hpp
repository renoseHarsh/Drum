#ifndef DRUM_FILESYSTEM_HPP
#define DRUM_FILESYSTEM_HPP

#include "file_tree/file_tree.hpp"
#include <optional>
#include <string>

namespace drum::fs {
  std::optional<std::string> build_file_tree(const ft::Node &file_tree);
}

#endif // !DRUM_FILESYSTEM_HPP
