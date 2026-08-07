module builder_cmd;

import std;

import :discover;
import :compile;
import :link;
import :archive;

import manifest;

namespace fs = std::filesystem;

namespace drum::builder_cmd {

  namespace {
    bool is_valid_exec_dir() {
      std::error_code ec{};
      return fs::is_directory("src", ec) &&
             fs::is_regular_file("src/main.cpp", ec);
    }

    bool is_valid_lib_dir() {
      std::error_code ec{};
      return fs::is_directory("src", ec) && fs::is_directory("include");
    }
  } // namespace

  std::expected<void, std::string> execute(const BuildArgs &,
                                           const manifest::Manifest &manifest) {

    switch (manifest.type) {
    case manifest::Manifest::Type::exec:
      if (!is_valid_exec_dir()) {
        return std::unexpected{"Invalid executable package layout"};
      }
      break;
    case manifest::Manifest::Type::lib:
      if (!is_valid_lib_dir()) {
        return std::unexpected{"Invalid executable package layout"};
      }
      break;
    }

    auto src_result = discover::discover();
    if (!src_result) {
      return std::unexpected{std::move(src_result).error()};
    }

    auto obj_result = compile::compile(*src_result, manifest);
    if (!obj_result) {
      return std::unexpected{std::move(obj_result).error()};
    }

    switch (manifest.type) {
    case manifest::Manifest::Type::exec:
      return link::link(*obj_result, manifest.name);
      break;
    case manifest::Manifest::Type::lib:
      return archive::archive(*obj_result, manifest.name);
      break;
    }
  } // namespace drum::builder_cmd
} // namespace drum::builder_cmd
