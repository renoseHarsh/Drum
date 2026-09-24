module mod_deps:comp_db;

import std;

import glaze;
import compiler_flags;

namespace fs = std::filesystem;

namespace drum::builder_cmd::mod_deps::comp_db {

  namespace {

    using SourceObject = std::pair<fs::path, fs::path>;

    [[nodiscard]] std::vector<std::string>
    generate_arguments(const compiler_flags::Flags &flags,
                       const SourceObject &source_object) {
      std::vector<std::string> args{};
      args.reserve(flags.args().size() + 5);

      args.push_back("clang++");
      args.append_range(flags.args());
      args.push_back("-c");
      args.push_back(source_object.first.string());
      args.push_back("-o");
      args.push_back(source_object.second.string());

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
  std::expected<fs::path, std::string>
  generate_compile_database(const std::vector<SourceObject> &source_objects,
                            const compiler_flags::Flags &flags,
                            const fs::path &output_dir) {
    fs::path comp_db_file{output_dir / "scan_deps_compile_commands.json"};

    std::vector<DatabaseEntry> database{};
    database.reserve(source_objects.size());

    std::ranges::transform(source_objects, std::back_inserter(database),
                           [&](const auto &source_object) {
                             return DatabaseEntry{
                                 .directory = ".",
                                 .arguments =
                                     generate_arguments(flags, source_object),
                                 .file = source_object.first,
                                 .output = source_object.second};
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

} // namespace drum::builder_cmd::mod_deps::comp_db
