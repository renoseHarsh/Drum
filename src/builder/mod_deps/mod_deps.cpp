module mod_deps;

import std;

import :comp_db;

import compiler_flags;

namespace fs = std::filesystem;

namespace drum::builder_cmd::mod_deps {

  [[nodiscard]]
  std::expected<void, std::string>
  scan_modules(const std::vector<SourceObject> &source_objects,
               const compiler_flags::Flags &flags, const fs::path &output_dir) {
    return comp_db::generate_compile_database(source_objects, flags, output_dir)
        .and_then(
            [](fs::path) -> std::expected<void, std::string> { return {}; });
  }

} // namespace drum::builder_cmd::mod_deps
