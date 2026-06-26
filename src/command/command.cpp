#include "command/command.hpp"
#include <print>

namespace command {
  void execute(const args::NewArgs &args) {
    std::println("Package name {}, Package Type {}", args.pkg_name,
                 args.pkg_type == args::NewArgs::PackageType::executable
                     ? "executable"
                     : "library");
  }
} // namespace command
