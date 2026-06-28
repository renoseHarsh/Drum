#ifndef DRUM_COMMANDS_HPP
#define DRUM_COMMANDS_HPP

#include <string>
namespace drum::commands {

  struct NewArgs {
    enum class PackageType { executable, library };

    std::string pkg_name;
    PackageType pkg_type = PackageType::executable;
  };

} // namespace drum::commands

#endif // !DRUM_COMMANDS_HPP
