#include "executor/executor.hpp"
#include <print>

namespace drum::executor {
  void execute(const drum::commands::NewArgs &cmd) {
    std::println("Package name {}, Package Type {}", cmd.pkg_name,
                 cmd.pkg_type ==
                         drum::commands::NewArgs::PackageType::executable
                     ? "executable"
                     : "library");
  }
} // namespace drum::executor
