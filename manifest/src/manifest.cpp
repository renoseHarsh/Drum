module manifest;

import std;

import tomlplusplus;

namespace drum::manifest {
  std::expected<Manifest, std::string> parse() {
    std::error_code ec{};
    if (!std::filesystem::exists("drum.toml", ec)) {
      return std::unexpected{"Missing drum.toml"};
    }
    const auto result = toml::parse_file("drum.toml");
    if (!result)
      return std::unexpected{std::string{result.error().description()}};

    const auto &t = result.table();

    if (!t.contains("name"))
      return std::unexpected{"Missing name"};
    auto name = t["name"].value<std::string>();
    if (!name)
      return std::unexpected{"Invalid name"};

    if (!t.contains("version"))
      return std::unexpected{"Missing version"};
    auto version = t["version"].value<std::string>();
    if (!version)
      return std::unexpected{"Invalid version"};

    if (!t.contains("type"))
      return std::unexpected{"Missing type"};
    const auto type_str = t["type"].value<std::string>();
    if (!type_str)
      return std::unexpected{"Invalid type"};
    const auto &type = type_str.value();

    if (type == "exec")
      return Manifest{std::move(name).value(), std::move(version).value(),
                      Type::exec};
    else if (type == "lib")
      return Manifest{std::move(name).value(), std::move(version).value(),
                      Type::lib};
    return std::unexpected{"Invalid type"};
  }
} // namespace drum::manifest
