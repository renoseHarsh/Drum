module manifest;

import std;

import tomlplusplus;

namespace fs = std::filesystem;

namespace drum::manifest {
  namespace {
    constexpr std::string_view invalid_error{"Invalid {}"};

    std::expected<std::string_view, std::string>
    extract_string(const toml::table &table, std::string_view key) {
      const auto *node = table.get(key);

      if (!node)
        return std::unexpected{std::format("Missing {}", key)};

      if (auto value = node->value<std::string_view>())
        return *value;

      return std::unexpected{std::format(invalid_error, key)};
    }

    std::expected<std::optional<std::string_view>, std::string>
    extract_optional(const toml::table &table, std::string_view key) {
      const auto *node = table.get(key);

      if (!node)
        return std::nullopt;

      if (auto value = node->value<std::string_view>())
        return *value;

      return std::unexpected{std::format(invalid_error, key)};
    }

    using namespace std::literals::string_view_literals;

    std::expected<Manifest::Standard, std::string>
    parse_standard(std::string_view standard) {
      constexpr std::array standards{
          std::pair{"c++11"sv, Manifest::Standard::cpp11},
          std::pair{"c++14"sv, Manifest::Standard::cpp14},
          std::pair{"c++17"sv, Manifest::Standard::cpp17},
          std::pair{"c++20"sv, Manifest::Standard::cpp20},
          std::pair{"c++23"sv, Manifest::Standard::cpp23},
          std::pair{"c++26"sv, Manifest::Standard::cpp26},
      };

      const auto it = std::ranges::find_if(standards, [&](const auto &entry) {
        return entry.first == standard;
      });

      if (it == standards.end())
        return std::unexpected{"Invalid standard"};

      return it->second;
    }

    std::expected<Manifest::Type, std::string>
    parse_type(std::string_view type) {
      constexpr std::array types{std::pair{"lib"sv, Manifest::Type::lib},
                                 std::pair{"exec"sv, Manifest::Type::exec}};

      auto it = std::ranges::find_if(
          types, [&](const auto &entry) { return entry.first == type; });

      if (it == types.end())
        return std::unexpected{"Invalid type"};

      return it->second;
    }

  } // namespace

  std::expected<Manifest, std::string> parse() {
    fs::path manifest_path{"drum.toml"};

    std::error_code ec{};
    if (!fs::exists(manifest_path, ec))
      return std::unexpected{"Missing drum.toml"};

    const auto manifest_file = toml::parse_file(manifest_path.string());
    if (!manifest_file)
      return std::unexpected{std::string{manifest_file.error().description()}};

    const auto &t = manifest_file.table();
    Manifest manifest{};

    {
      auto result = extract_string(t, "name");
      if (!result)
        return std::unexpected{std::move(result).error()};

      manifest.name = std::string{*result};
    }

    {
      auto result = extract_string(t, "version");
      if (!result)
        return std::unexpected{std::move(result).error()};

      manifest.version = std::string{*result};
    }

    {
      auto result = extract_string(t, "type").and_then(parse_type);
      if (!result)
        return std::unexpected{std::move(result).error()};

      manifest.type = *result;
    }

    if (auto build_table = t["build"].as_table()) {
      auto result = extract_optional(*build_table, "standard");
      if (!result)
        return std::unexpected{std::move(result).error()};

      if (*result) {
        auto standard = parse_standard(**result);
        if (!standard)
          return std::unexpected{std::move(standard.error())};

        manifest.standard = *standard;
      }
    }

    manifest.timestamp = fs::last_write_time(manifest_path, ec);
    if (ec) {
      return std::unexpected{
          std::format("Failed to read drum.toml timestamp: {}", ec.message())};
    }

    return manifest;
  }
} // namespace drum::manifest
