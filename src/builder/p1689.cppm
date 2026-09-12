module builder_cmd:p1689;

import std;

import :compiler;

import glaze;

namespace fs = std::filesystem;

namespace drum::builder_cmd::p1689 {
  using SourceObject = std::pair<fs::path, fs::path>;

  namespace {
    [[nodiscard]]
    std::vector<std::string>
    generate_arguments(const compiler::Compiler &compiler,
                       const SourceObject &source_object) {
      std::vector<std::string> args;

      args.reserve(compiler.args().size() + 5);
      args.push_back("clang++");
      args.append_range(compiler.args());
      args.push_back("-c");
      args.push_back(source_object.first);
      args.push_back("-o");
      args.push_back(source_object.second);

      return args;
    }
  } // namespace

  struct DatabaseEntry {
    std::string directory{};
    std::vector<std::string> arguments{};
    std::string file{};
    std::string output{};
  };

  [[nodiscard]]
  std::expected<fs::path, std::string>
  generate(const std::vector<SourceObject> &source_objects,
           const compiler::Compiler &compiler, const fs::path &output_dir) {
    fs::path p1689_file{output_dir / "p1689.json"};
    std::vector<DatabaseEntry> database;
    database.reserve(source_objects.size());

    std::ranges::transform(
        source_objects, std::back_inserter(database),
        [&](const auto &source_object) {
          return DatabaseEntry{.directory = ".",
                               .arguments =
                                   generate_arguments(compiler, source_object),
                               .file = source_object.first,
                               .output = source_object.second};
        });

    std::string buffer;
    auto result = glaze::write_file_json(database, p1689_file.string(), buffer);

    if (result) [[unlikely]] {
      buffer.clear();
      return std::unexpected{glaze::format_error(result, buffer)};
    }
    return p1689_file;
  }
} // namespace drum::builder_cmd::p1689
