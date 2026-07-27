module builder_cmd:validate;

import std;

import tomlplusplus;

namespace drum::builder_cmd::validate {
  enum class Type { lib, exec };
  namespace {
    std::expected<Type, std::string> parse_manifest() {
      const auto result = toml::parse_file("drum.toml");
      if (!result) {
        return std::unexpected{std::string{result.error().description()}};
      }

      const auto table = std::move(result).table();

      if (!table.contains("name")) {
        return std::unexpected{"Missing name"};
      }

      if (!table.contains("version")) {
        return std::unexpected{"Missing version"};
      }

      if (!table.contains("type")) {
        return std::unexpected{"Missing type"};
      }

      const auto type_str = table["type"].value<std::string_view>();
      if (!type_str) {
        return std::unexpected{"Invalid type"};
      }

      if (type_str.value() == "exec") {
        return Type::exec;
      } else if (type_str.value() == "lib") {
        return Type::lib;
      }

      return std::unexpected{"Invalid type"};
    }

    bool is_valid_exec_dir() {
      return std::filesystem::is_directory("src") &&
             std::filesystem::is_regular_file("src/main.cpp");
    }
  } // namespace

  std::expected<Type, std::string> validate() {
    if (!std::filesystem::exists("drum.toml")) {
      return std::unexpected{"Missing drum.toml"};
    }

    const auto type_result = parse_manifest();
    if (!type_result) {
      return type_result;
    }

    switch (type_result.value()) {
    case Type::exec:
      if (!is_valid_exec_dir()) {
        return std::unexpected{"Invalid executable package layout"};
      }
      break;
    case Type::lib:
      return std::unexpected{"Not implemented lib"};
      break;
    }

    return type_result;
  }
} // namespace drum::builder_cmd::validate
