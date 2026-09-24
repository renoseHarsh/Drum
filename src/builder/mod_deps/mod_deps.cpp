module mod_deps;

import std;

import :comp_db;
import :scan_deps;

import compiler_flags;

namespace fs = std::filesystem;

namespace drum::builder_cmd::mod_deps {

  [[nodiscard]]
  std::expected<void, std::string>
  scan_modules(const std::vector<SourceObject> &source_objects,
               const compiler_flags::Flags &flags, const fs::path &output_dir) {
    return comp_db::generate_compile_database(source_objects, flags, output_dir)
        .and_then([](fs::path comp_db_file) {
          return scan_deps::scan_deps(comp_db_file);
        })
        .and_then([](std::string json_string) {
          return scan_deps::parse(json_string);
        })
        .and_then([](auto) -> std::expected<void, std::string> { return {}; });
  }

} // namespace drum::builder_cmd::mod_deps
