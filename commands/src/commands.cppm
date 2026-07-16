export module commands;
import std;

export namespace drum::commands {
  struct NewArgs {
    enum class PackageType { executable, library };
    std::string pkg_name;
    PackageType pkg_type = PackageType::executable;
  };

} // namespace drum::commands
