#ifndef ARGS_HPP
#define ARGS_HPP

#include <string>
namespace args {

  struct NewArgs {
    enum class PackageType { executable, library };

    std::string pkg_name;
    PackageType pkg_type = PackageType::executable;
  };

} // namespace args

#endif // !ARGS_HPP
