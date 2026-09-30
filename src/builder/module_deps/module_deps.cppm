export module module_deps;

import std;

export import compiler_flags;
export import compile_unit;

namespace fs = std::filesystem;

export namespace drum::builder_cmd::module_deps {

  [[nodiscard]]
  std::expected<std::vector<compile_unit::TranslationUnit>, std::string>
  order(std::vector<compile_unit::TranslationUnit> units,
        const compiler_flags::Flags &flags, const fs::path &output_dir);

} // namespace drum::builder_cmd::module_deps
