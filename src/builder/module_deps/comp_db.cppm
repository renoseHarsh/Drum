module module_deps:comp_db;

import std;

import glaze;
import compiler_flags;
import compile_unit;

namespace fs = std::filesystem;

namespace drum::builder_cmd::module_deps::comp_db {

  namespace {

    [[nodiscard]] std::vector<std::string>
    generate_arguments(const compiler_flags::Flags &flags,
                       const compile_unit::TranslationUnit &unit) {
      std::vector<std::string> args{};
      args.reserve(flags.args().size() + 5);

      args.push_back("clang++");
      args.append_range(flags.args());
      args.push_back("-c");
      args.push_back(unit.source.string());
      args.push_back("-o");
      args.push_back(unit.object.string());

      return args;
    }

  }; // namespace

  struct DatabaseEntry {
    std::string directory{};
    std::vector<std::string> arguments{};
    std::string file{};
    std::string output{};
  };

  [[nodiscard]]
  std::expected<fs::path, std::string> generate_compile_database(
      const std::vector<compile_unit::TranslationUnit> &units,
      const compiler_flags::Flags &flags, const fs::path &output_dir) {
    fs::path comp_db_file{output_dir / "scan_deps_compile_commands.json"};

    std::vector<DatabaseEntry> database{};
    database.reserve(units.size());

    std::ranges::transform(
        units, std::back_inserter(database), [&](const auto &unit) {
          return DatabaseEntry{.directory = ".",
                               .arguments = generate_arguments(flags, unit),
                               .file = unit.source.string(),
                               .output = unit.object.string()};
        });

    std::string buffer;
    auto error =
        glaze::write_file_json(database, comp_db_file.string(), buffer);

    if (error) [[unlikely]] {
      buffer.clear();
      return std::unexpected{glaze::format_error(error, buffer)};
    }

    return comp_db_file;
  }

} // namespace drum::builder_cmd::module_deps::comp_db
