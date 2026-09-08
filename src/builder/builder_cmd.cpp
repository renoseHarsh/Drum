module builder_cmd;

import std;

import :archive;
import :compile;
import :compiler;
import :discover;
import :link;
import :p1689;

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

  std::expected<void, std::string> execute(const BuildArgs &args,
                                           const manifest::Manifest &manifest) {
    compiler::Compiler compiler{};

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
      compiler.add_include_directory("include");
      break;
    }

    const auto &build = manifest.build;
    compiler.set_standard(build.standard)
        .set_warnings(build.warnings)
        .set_warnings_as_errors(build.warnings_as_errors)
        .set_extra_flags(build.extra_flags);

    const auto &profile = args.release ? manifest.release : manifest.debug;
    compiler.set_optimization(profile.optimization).set_debug(profile.debug);
    const fs::path profile_dir{args.release ? "release" : "debug"};
    const fs::path output_dir{"build" / profile_dir};
    fs::path output_path{output_dir / manifest.name};

    if (manifest.type == manifest::Manifest::Type::lib)
      output_path.replace_extension(".a");

    std::error_code ec;
    fs::create_directories(output_dir, ec);
    if (ec)
      return std::unexpected{
          std::format("Failed to create output directory '{}': {}",
                      output_dir.string(), ec.message())};

    return discover::discover()

        .transform([&](std::vector<fs::path> srcs) {
          return srcs |
                 std::views::transform([&](auto src) {
                   fs::path obj{output_dir};
                   obj /= src.lexically_relative("src/");
                   obj.replace_extension(".o");
                   return std::pair{std::move(src), obj};
                 }) |
                 std::ranges::to<std::vector>();
        })

        .and_then([&](std::vector<compile::SourceObject> source_objects)
                      -> std::expected<std::vector<compile::SourceObject>,
                                       std::string> {
          return p1689::generate(source_objects, compiler,
                                 output_dir / "p1689.json")
              .transform([&]() { return std::move(source_objects); });
        })

        .and_then([&](std::vector<compile::SourceObject> source_objects) {
          return compile::compile(source_objects, compiler, manifest.timestamp);
        })

        .and_then([&](std::vector<fs::path> objs) {
          switch (manifest.type) {
          case manifest::Manifest::Type::exec:
            return link::link(objs, output_path);

          case manifest::Manifest::Type::lib:
            return archive::archive(objs, output_path);
          }

          std::unreachable();
        });
  } // namespace drum::builder_cmd
} // namespace drum::builder_cmd
