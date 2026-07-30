module builder_cmd;

import std;

import :discover;
import :compile;
import :link;

import manifest;

namespace drum::builder_cmd {

  namespace {
    bool is_valid_exec_dir() {
      std::error_code ec{};
      return std::filesystem::is_directory("src", ec) &&
             std::filesystem::is_regular_file("src/main.cpp", ec);
    }
  } // namespace

  std::expected<void, std::string> execute(const BuildArgs &,
                                           const manifest::Manifest &manifest) {

    switch (manifest.type) {
    case manifest::Type::exec:
      if (!is_valid_exec_dir()) {
        return std::unexpected{"Invalid executable package layout"};
      }
      break;
    case manifest::Type::lib:
      return std::unexpected{"Not implemented lib"};
      break;
    }

    const auto src_result = discover::discover();

    auto obj_result = compile::compile(src_result);
    if (!obj_result) {
      return std::unexpected{std::move(obj_result).error()};
    }

    if (auto result = link::link(obj_result.value(), manifest.name); !result) {
      return std::unexpected{std::move(result).error()};
    }

    return {};
  }
} // namespace drum::builder_cmd
