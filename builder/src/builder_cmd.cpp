module builder_cmd;

import std;

import :discover;
import :compile;
import :link;
import :archive;
import :compiler;

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

    return discover::discover()
        .and_then([&](std::vector<fs::path> srcs) {
          return compile::compile(srcs, compiler, manifest.timestamp);
        })
        .and_then([&](std::vector<fs::path> objs) {
          switch (manifest.type) {
          case manifest::Manifest::Type::exec:
            return link::link(objs, manifest.name);

          case manifest::Manifest::Type::lib:
            return archive::archive(objs, manifest.name);
          }

          std::unreachable();
        });
  } // namespace drum::builder_cmd
} // namespace drum::builder_cmd
