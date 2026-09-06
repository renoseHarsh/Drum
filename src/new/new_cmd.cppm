export module new_cmd;

import std;

export namespace drum::new_cmd {
  struct NewArgs {
    enum class PackageType { executable, library };
    std::string pkg_name;
    PackageType pkg_type{PackageType::executable};
  };

  std::expected<void, std::string> execute(const NewArgs &args);
} // namespace drum::new_cmd
