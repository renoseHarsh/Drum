module;

#include <glaze/glaze.hpp>

export module glaze:p1689;

import std;

export namespace drum::glaze::p1689_types {

  struct ModuleDependency {
    std::string logical_name;
    std::optional<std::string> source_path;
  };

  struct ProvidedModule {
    bool is_interface;
    std::string logical_name;
    std::optional<std::string> source_path;
  };

  struct Rule {
    std::string primary_output;
    std::optional<std::vector<ProvidedModule>> provides;
    std::optional<std::vector<ModuleDependency>> requires_modules;
  };

  struct DependencyInfo {
    int revision;
    std::vector<Rule> rules;
    int version;
  };

} // namespace drum::glaze::p1689_types

template <>
struct glz::meta<drum::glaze::p1689_types::ModuleDependency> : glz::kebab_case {
};

template <>
struct glz::meta<drum::glaze::p1689_types::ProvidedModule> : glz::kebab_case {};

template <> struct glz::meta<drum::glaze::p1689_types::Rule> {
  static constexpr auto value = glz::object(
      "primary-output", &drum::glaze::p1689_types::Rule::primary_output,
      "provides", &drum::glaze::p1689_types::Rule::provides, "requires",
      &drum::glaze::p1689_types::Rule::requires_modules);
};

template <>
struct glz::meta<drum::glaze::p1689_types::DependencyInfo> : glz::kebab_case {};
