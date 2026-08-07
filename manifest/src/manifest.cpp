module manifest;

import std;

import tomlplusplus;

namespace fs = std::filesystem;

namespace drum::manifest {
  namespace {
    constexpr std::string_view invalid_error{"Invalid {}"};

    std::expected<std::string_view, std::string>
    extract_string(const toml::table &table, std::string_view key) {
      if (!table.contains(key))
        return std::unexpected{std::format("Missing {}", key)};

      if (auto result = table[key].value<std::string_view>()) {
        return result.value();
      }

      return std::unexpected{std::format(invalid_error, key)};
    }

  } // namespace

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

    Manifest manifest{};

    if (auto name_result = extract_string(t, "name"))
      manifest.name = std::string{name_result.value()};
    else
      return std::unexpected{std::move(name_result).error()};

    if (auto version_result = extract_string(t, "version"))
      manifest.version = std::string{version_result.value()};
    else
      return std::unexpected{std::move(version_result).error()};

    if (auto type_result = extract_string(t, "type").and_then(
            [](std::string_view type)
                -> std::expected<Manifest::Type, std::string> {
              if (type == "exec")
                return Manifest::Type::exec;
              else if (type == "lib")
                return Manifest::Type::lib;

              return std::unexpected{"Invalid type"};
            })) {
      manifest.type = type_result.value();
    } else {
      return std::unexpected{std::move(type_result).error()};
    }


    auto timestamp = fs::last_write_time(manifest_path, ec);
    if (ec) {
      return std::unexpected{
          std::format("Failed to read drum.toml timestamp: {}", ec.message())};
    }
    manifest.timestamp = std::move(timestamp);

    return manifest;
  }
} // namespace drum::manifest
