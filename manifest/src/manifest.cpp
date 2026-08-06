module manifest;

import std;

import tomlplusplus;

namespace fs = std::filesystem;

namespace drum::manifest {
  std::expected<std::string_view, std::string>
  extract_string(const toml::table &table, std::string_view key) {
    if (!table.contains(key))
      return std::unexpected{std::format("Missing {}", key)};

    auto result = table[key].value<std::string_view>();
    if (!result)
      return std::unexpected{std::format("Invalid {}", key)};

    return result.value();
  }

  std::expected<Manifest, std::string> parse() {
    fs::path manifest_path{"drum.toml"};
    std::error_code ec{};
    if (!fs::exists(manifest_path, ec)) {
      return std::unexpected{"Missing drum.toml"};
    }
    const auto manifest_file = toml::parse_file("drum.toml");
    if (!manifest_file)
      return std::unexpected{std::string{manifest_file.error().description()}};

    const auto &t = manifest_file.table();

    auto name_result = extract_string(t, "name");
    if (!name_result) {
      return std::unexpected{std::move(name_result).error()};
    }

    auto version_result = extract_string(t, "version");
    if (!version_result) {
      return std::unexpected{std::move(version_result).error()};
    }

    auto type_result = extract_string(t, "type");
    if (!type_result) {
      return std::unexpected{std::move(type_result).error()};
    }

    auto timestamp = fs::last_write_time(manifest_path, ec);
    if (ec) {
      return std::unexpected{
          std::format("Failed to read drum.toml timestamp: {}", ec.message())};
    }

    Type type;
    if (type_result.value() == "exec") {
      type = Type::exec;
    } else if (type_result.value() == "lib") {
      type = Type::lib;
    } else {
      return std::unexpected{"Invalid type"};
    }

    return Manifest{std::string{name_result.value()},
                    std::string{version_result.value()}, type,
                    std::move(timestamp)};
  }
} // namespace drum::manifest
