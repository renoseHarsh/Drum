module manifest;

import std;

import tomlplusplus;

namespace fs = std::filesystem;

namespace drum::manifest {
  namespace {
    constexpr std::string_view invalid_error{"Invalid {}"};

    std::expected<std::string_view, std::string>
    extract_string(const toml::table &table, std::string_view key) {
      if (auto node = table.get(key)) {
        if (auto value = node->value<std::string_view>())
          return *value;
        return std::unexpected{std::format(invalid_error, key)};
      }
      return std::unexpected{std::format("Missing {}", key)};
    }

    std::expected<std::optional<std::string_view>, std::string>
    extract_optional(const toml::table &table, std::string_view key) {

      if (auto node = table.get(key)) {
        if (auto value = node->value<std::string_view>())
          return *value;
        return std::unexpected{std::format(invalid_error, key)};
      }
      return std::nullopt;
    }

    std::expected<Manifest::Standard, std::string>
    parse_standard(std::string_view standard) {
      constexpr std::array table{
          std::pair{std::string_view{"c++11"}, Manifest::Standard::cpp11},
          std::pair{std::string_view{"c++14"}, Manifest::Standard::cpp14},
          std::pair{std::string_view{"c++17"}, Manifest::Standard::cpp17},
          std::pair{std::string_view{"c++20"}, Manifest::Standard::cpp20},
          std::pair{std::string_view{"c++23"}, Manifest::Standard::cpp23},
          std::pair{std::string_view{"c++26"}, Manifest::Standard::cpp26},
      };

      auto it = std::ranges::find_if(
          table, [&](const auto &entry) { return entry.first == standard; });

      if (it != table.end())
        return it->second;

      return std::unexpected{"Invalid standard"};
    }

    std::expected<Manifest::Type, std::string>
    parse_type(std::string_view type) {
      constexpr std::array table{
          std::pair{std::string_view{"lib"}, Manifest::Type::lib},
          std::pair{std::string_view{"exec"}, Manifest::Type::exec}};

      auto it = std::ranges::find_if(
          table, [&](const auto &entry) { return entry.first == type; });

      if (it != table.end())
        return it->second;

      return std::unexpected{"Invalid type"};
    }

  } // namespace

  std::expected<Manifest, std::string> parse() {
    fs::path manifest_path{"drum.toml"};
    std::error_code ec{};
    if (!fs::exists(manifest_path, ec)) {
      return std::unexpected{"Missing drum.toml"};
    }

    const auto manifest_file = toml::parse_file(manifest_path.string());
    if (!manifest_file)
      return std::unexpected{std::string{manifest_file.error().description()}};

    const auto &t = manifest_file.table();

    Manifest manifest{};

    if (auto name_result = extract_string(t, "name"))
      manifest.name = std::string{*name_result};
    else
      return std::unexpected{std::move(name_result).error()};

    if (auto version_result = extract_string(t, "version"))
      manifest.version = std::string{*version_result};
    else
      return std::unexpected{std::move(version_result).error()};

    if (auto type_result = extract_string(t, "type").and_then(parse_type)) {
      manifest.type = *type_result;
    } else
      return std::unexpected{std::move(type_result).error()};

    if (auto build_table = t["build"].as_table()) {
      if (auto standard = extract_optional(*build_table, "standard")) {
        if (standard->has_value()) {
          auto parsed = parse_standard(**standard);
          if (!parsed)
            return std::unexpected{std::move(parsed).error()};

          manifest.standard = *parsed;
        }
      } else {
        return std::unexpected{std::move(standard.error())};
      }
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
