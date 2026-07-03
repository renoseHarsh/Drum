#ifndef DRUM_BLUEPRINT_HPP
#define DRUM_BLUEPRINT_HPP

#include "file_tree/file_tree.hpp"
#include <string_view>

namespace drum::bp {
  ft::Node create_new_package(std::string_view pkg_name, bool lib);
} // namespace drum::bp

#endif // !DRUM_BLUEPRINT_HPP
