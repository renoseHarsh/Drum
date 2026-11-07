module builder_cmd;

import std;

import :archive;
import :compile;
import :discover;
import :link;

import manifest;
import compiler_flags;
import module_deps;

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
    compiler_flags::Flags flags{};

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
      flags.add_include_directory("include");
      break;
    }

    if (!manifest.profiles.contains(args.profile))
      return std::unexpected{std::format("Unknown profile: {}", args.profile)};

    const fs::path output_dir = fs::path{"build"} / args.profile;

    fs::path output_path{output_dir / manifest.name};
    if (manifest.type == manifest::Manifest::Type::lib)
      output_path.replace_extension(".a");

    flags.set_standard(manifest.build.standard)
        .add_flags(manifest.build.flags)
        .add_flags(manifest.profiles.at(args.profile));

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
                   obj.replace_extension(src.extension().string() + ".o");
                   return compile_unit::TranslationUnit{std::move(src),
                                                        std::move(obj)};
                 }) |
                 std::ranges::to<std::vector>();
        })
        .and_then([&](auto units) {
          return module_deps::order(std::move(units), flags, output_dir);
        })

        .and_then([&](auto units) {
          return compile::compile(units, flags, manifest.timestamp);
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
