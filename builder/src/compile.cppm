module builder_cmd:compile;

import std;

import :process;
import :dependency;
import :log;

import manifest;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile {

  namespace {
    std::string_view
    get_standard(manifest::Manifest::Build::Standard standard) {
      switch (standard) {
        using enum manifest::Manifest::Build::Standard;
      case cpp11:
        return "c++11";
      case cpp14:
        return "c++14";
      case cpp17:
        return "c++17";
      case cpp20:
        return "c++20";
      case cpp23:
        return "c++23";
      case cpp26:
        return "c++26";
      }
    }

    std::span<const std::string_view>
    get_warnings(manifest::Manifest::Build::Warnings warnings) {
      using enum manifest::Manifest::Build::Warnings;

      static constexpr std::array<std::string_view, 1> warnings_none{"-w"};
      static constexpr std::array<std::string_view, 2> warnings_all{"-Wall",
                                                                    "-Wextra"};
      static constexpr std::array<std::string_view, 3> warnings_pedantic{
          "-Wall", "-Wextra", "-Wpedantic"};

      switch (warnings) {
      case none:
        return warnings_none;
      case default_:
        return {};
      case all:
        return warnings_all;
      case pedantic:
        return warnings_pedantic;
      }
    }

    std::expected<void, std::string>
    compile_source(const fs::path &src, const fs::path &obj,
                   const manifest::Manifest &manifest) {
      std::vector<std::string> args{
          "clang++", "-c", src.string(), "-Isrc/", "-o", obj.string(), "-MMD"};

      if (manifest.type == manifest::Manifest::Type::lib) {
        args.push_back("-Iinclude/");
      }

      args.push_back(
          std::format("-std={}", get_standard(manifest.build.standard)));

      args.append_range(get_warnings(manifest.build.warnings) |
                        std::views::transform(
                            [](auto warning) { return std::string{warning}; }));

      if (manifest.build.warnings_as_errors)
        args.push_back("-Werror");

      args.append_range(manifest.build.extra_flags);

      return process::run_process(args);
    }

    bool object_is_stale(const fs::path &object,
                         const fs::file_time_type &manifest_timestamp) {
      std::error_code ec{};
      const auto last_write_obj = fs::last_write_time(object, ec);
      if (ec)
        return true;

      if (manifest_timestamp > last_write_obj)
        return true;

      auto dependency = object;
      dependency.replace_extension(".d");

      const auto dependencies_result = dependency::get_dependencies(dependency);
      if (!dependencies_result)
        return true;

      const auto &[target, dependencies] = *dependencies_result;

      if (object != target)
        return true;

      return std::ranges::any_of(dependencies, [&](const auto &dep) {
        const auto last_dep_write = fs::last_write_time(dep, ec);
        return ec || (last_dep_write > last_write_obj);
      });
    }
  } // namespace

  std::expected<std::vector<fs::path>, std::string>
  compile(const std::vector<fs::path> &sources,
          const manifest::Manifest &manifest) {
    std::vector<fs::path> objects{};
    objects.reserve(sources.size());

    for (const auto &source : sources) {
      fs::path obj{"build"};
      obj /= source.lexically_relative("src/");
      obj.replace_extension(".o");

      std::error_code ec;
      if (!fs::exists(obj, ec) || object_is_stale(obj, manifest.timestamp)) {
        log::compile(obj);
        fs::create_directories(obj.parent_path(), ec);
        if (ec)
          return std::unexpected{"Error in creating build directory"};

        if (auto result = compile_source(source, obj, manifest); !result)
          return std::unexpected{std::move(result).error()};

      } else
        log::cache_hit(obj);

      objects.push_back(std::move(obj));
    }

    return objects;
  }
} // namespace drum::builder_cmd::compile
