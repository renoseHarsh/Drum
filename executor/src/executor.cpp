#include "blueprint/blueprint.hpp"
#include "commands/commands.hpp"
#include "executor/executor.hpp"
#include "filesystem/filesystem.hpp"
#include <optional>
#include <string>

namespace drum::executor {
  std::optional<std::string> execute(const commands::NewArgs &cmd) {
    auto package = bp::create_new_package(
        cmd.pkg_name, cmd.pkg_type == commands::NewArgs::PackageType::library);
    if (auto error_msg = fs::build_file_tree(package)) {
      return error_msg.value();
    }
    return std::nullopt;
  }
} // namespace drum::executor
