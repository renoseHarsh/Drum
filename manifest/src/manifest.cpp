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

    template <typename T>
    std::expected<std::optional<T>, std::string>
    extract_optional(const toml::table &table, std::string_view key) {
      const auto *node = table.get(key);

      if (!node)
        return std::nullopt;

      if constexpr (std::same_as<T, std::string_view>) {
        if (!node->is_string())
          return std::unexpected{std::format(invalid_error, key)};
      } else if (!node->is<T>())
        return std::unexpected{std::format(invalid_error, key)};

      return node->value<T>().value();
    }

    template <typename T>
    std::expected<std::optional<std::vector<T>>, std::string>
    extract_optional_array(const toml::table &table, std::string_view key) {
      const auto *node = table.get(key);

      if (!node)
        return std::nullopt;

      if (!node->is_array())
        return std::unexpected{std::format(invalid_error, key)};

      const auto *array = node->as_array();

      const bool valid = std::ranges::all_of(
          *array, [](const auto &node) { return node.template is<T>(); });

      if (!valid)
        return std::unexpected{std::format(invalid_error, key)};

      std::vector<T> result;
      result.reserve(array->size());

      std::ranges::transform(
          *array, std::back_inserter(result),
          [](const auto &node) { return node.template value<T>().value(); });

      return result;
    }

    using namespace std::literals::string_view_literals;

    std::expected<Manifest::Build::Standard, std::string>
    parse_standard(std::string_view standard) {
      using enum manifest::Manifest::Build::Standard;
      constexpr std::array standards{
          std::pair{"c++11"sv, cpp11}, std::pair{"c++14"sv, cpp14},
          std::pair{"c++17"sv, cpp17}, std::pair{"c++20"sv, cpp20},
          std::pair{"c++23"sv, cpp23}, std::pair{"c++26"sv, cpp26},
      };

      const auto it = std::ranges::find_if(standards, [&](const auto &entry) {
        return entry.first == standard;
      });

      if (it == standards.end())
        return std::unexpected{std::format(invalid_error, "standard")};

      return it->second;
    }

    std::expected<Manifest::Type, std::string>
    parse_type(std::string_view type) {
      constexpr std::array types{std::pair{"lib"sv, Manifest::Type::lib},
                                 std::pair{"exec"sv, Manifest::Type::exec}};

      auto it = std::ranges::find_if(
          types, [&](const auto &entry) { return entry.first == type; });

      if (it == types.end())
        return std::unexpected{std::format(invalid_error, "type")};

      return it->second;
    }

    std::expected<Manifest::Build::Warnings, std::string>
    parse_warnings(std::string_view warnings) {
      using enum manifest::Manifest::Build::Warnings;
      constexpr std::array levels{
          std::pair{"none"sv, none},
          std::pair{"default"sv, default_},
          std::pair{"all"sv, all},
          std::pair{"pedantic"sv, pedantic},
      };

      const auto it = std::ranges::find_if(
          levels, [&](const auto &entry) { return entry.first == warnings; });

      if (it == levels.end())
        return std::unexpected{std::format(invalid_error, "warnings level")};

      return it->second;
    }

    std::expected<Manifest::Build, std::string>
    parse_build(const toml::table &table) {
      Manifest::Build build{};

      {
        auto result =
            extract_optional<std::string_view>(table, "standard")
                .and_then(
                    [](auto value) -> std::expected<
                                       std::optional<Manifest::Build::Standard>,
                                       std::string> {
                      if (!value)
                        return std::nullopt;

                      return parse_standard(*value);
                    });

        if (!result)
          return std::unexpected{std::move(result).error()};

        if (*result)
          build.standard = **result;
      }

      {
        auto result =
            extract_optional<std::string_view>(table, "warnings")
                .and_then(
                    [](auto value) -> std::expected<
                                       std::optional<Manifest::Build::Warnings>,
                                       std::string> {
                      if (!value)
                        return std::nullopt;

                      return parse_warnings(*value);
                    });

        if (!result)
          return std::unexpected{std::move(result).error()};

        if (*result)
          build.warnings = **result;
      }

      {
        auto result = extract_optional<bool>(table, "warnings_as_errors");

        if (!result)
          return std::unexpected{std::move(result).error()};

        if (*result)
          build.warnings_as_errors = **result;
      }

      {
        auto result = extract_optional_array<std::string>(table, "extra_flags");

        if (!result)
          return std::unexpected{std::move(result).error()};

        if (*result)
          build.extra_flags = std::move(**result);
      }

      return build;
    }

    std::expected<Manifest::Profile::Optimization, std::string>
    parse_optimization(std::int64_t value) {
      if (value < 0 || value > 3)
        return std::unexpected{std::format(invalid_error, "optimization")};

      return static_cast<Manifest::Profile::Optimization>(value);
    }

    std::expected<Manifest::Profile, std::string>
    parse_profile(const toml::table &table, Manifest::Profile profile) {
      {
        auto result =
            extract_optional<std::int64_t>(table, "optimization")
                .and_then(
                    [](auto value)
                        -> std::expected<
                            std::optional<Manifest::Profile::Optimization>,
                            std::string> {
                      if (!value)
                        return std::nullopt;

                      return parse_optimization(*value);
                    });

        if (!result)
          return std::unexpected{std::move(result).error()};

        if (*result)
          profile.optimization = **result;
      }

      {
        auto result = extract_optional<bool>(table, "debug");

        if (!result)
          return std::unexpected{std::move(result).error()};

        if (*result)
          profile.debug = **result;
      }

      return profile;
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

    if (const auto build_table = t["build"].as_table()) {
      {
        auto result = parse_build(*build_table);
        if (!result)
          return std::unexpected{std::move(result).error()};

        manifest.build = std::move(*result);
      }
    }

    if (const auto profile_table = t["profile"].as_table()) {
      if (auto debug_table = (*profile_table)["debug"].as_table()) {
        auto result = parse_profile(*debug_table, manifest.debug);

        if (!result)
          return std::unexpected{std::move(result).error()};

        manifest.debug = std::move(*result);
      }

      if (auto debug_table = (*profile_table)["release"].as_table()) {
        auto result = parse_profile(*debug_table, manifest.release);

        if (!result)
          return std::unexpected{std::move(result).error()};

        manifest.release = std::move(*result);
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
