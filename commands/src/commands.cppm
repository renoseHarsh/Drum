export module commands;
import std;

namespace drum::commands {

  export struct NewArgs {
    enum class PackageType { executable, library };

    std::string pkg_name;
    PackageType pkg_type = PackageType::executable;
  };

} // namespace drum::commands
