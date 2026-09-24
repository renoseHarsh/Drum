export module mod_deps;

import std;

export import compiler_flags;

namespace fs = std::filesystem;

export namespace drum::builder_cmd::mod_deps {

  using SourceObject = std::pair<fs::path, fs::path>;

  [[nodiscard]]
  std::expected<void, std::string>
  scan_modules(const std::vector<SourceObject> &source_objects,
               const compiler_flags::Flags &flags, const fs::path &output_dir);

} // namespace drum::builder_cmd::mod_deps
