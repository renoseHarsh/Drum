module manifest;

import std;

import tomlplusplus;

namespace fs = std::filesystem;
using namespace std::literals::string_view_literals;

namespace drum::manifest {
  namespace {
    constexpr std::string_view invalid_error{"Invalid {}"};

    [[nodiscard]]
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
    [[nodiscard]]
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
    [[nodiscard]]
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

    [[nodiscard]]
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

    [[nodiscard]]
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

    [[nodiscard]]
    std::expected<void, std::string> fill_standard(const toml::table &table,
                                                   Manifest::Build &build) {
      auto standard = extract_optional<std::string_view>(table, "standard");

      if (!standard)
        return std::unexpected{std::move(standard).error()};

      if (*standard) {
        auto parsed = parse_standard(**standard);
        if (!parsed)
          return std::unexpected{std::move(parsed).error()};
        build.standard = *parsed;
      }

      return {};
    }

    [[nodiscard]]
    std::expected<void, std::string> fill_flags(const toml::table &table,
                                                Manifest::Build &build) {
      auto flags = extract_optional_array<std::string>(table, "flags");

      if (!flags)
        return std::unexpected{std::move(flags).error()};

      if (*flags)
        build.flags = std::move(**flags);

      return {};
    }

    [[nodiscard]]
    std::expected<Manifest::Build, std::string>
    parse_build(const toml::table &table) {
      Manifest::Build build{};

      auto result = fill_standard(table, build).and_then([&]() {
        return fill_flags(table, build);
      });

      if (!result)
        return std::unexpected{std::move(result).error()};

      return build;
    }

    [[nodiscard]]
    std::expected<void, std::string> fill_profiles(const toml::table &profiles,
                                                   Manifest &manifest) {
      for (const auto &[key, value] : profiles) {
        const auto flags = value.as_array();

        if (!flags)
          return std::unexpected{std::format(invalid_error, key.str())};

        const bool valid = std::ranges::all_of(
            *flags, [](const auto &flag) { return flag.is_string(); });
        if (!valid)
          return std::unexpected{std::format(invalid_error, key.str())};

        std::vector<std::string> flags_vec;
        flags_vec.reserve(flags->size());

        std::ranges::transform(
            *flags, std::back_inserter(flags_vec),
            [](const auto &node) { return node.as_string()->get(); });

        manifest.profiles[std::string{key}] = std::move(flags_vec);
      }

      return {};
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

      manifest.type = std::move(*result);
    }

    if (const auto build_table = t["build"].as_table()) {
      auto result = parse_build(*build_table);
      if (!result)
        return std::unexpected{std::move(result).error()};

      manifest.build = std::move(*result);
    }

    if (const auto profile_table = t["profiles"].as_table()) {
      auto result = fill_profiles(*profile_table, manifest);
      if (!result)
        return std::unexpected{std::move(result).error()};
    }

    manifest.timestamp = fs::last_write_time(manifest_path, ec);
    if (ec) {
      return std::unexpected{
          std::format("Failed to read drum.toml timestamp: {}", ec.message())};
    }

    return manifest;
  }
} // namespace drum::manifest
