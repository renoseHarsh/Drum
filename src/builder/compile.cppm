module builder_cmd:compile;

import std;

import :process;
import :dependency;
import :log;
import :compiler;

import manifest;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile {

  namespace {
    std::expected<void, std::string>
    compile_source(const fs::path &src, const fs::path &obj,
                   const compiler::Compiler &compiler) {
      std::array invocation{std::string("-c"), src.string(), std::string{"-o"},
                            obj.string()};
      return process::run_process("clang++", invocation, compiler.args());
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

  using SourceObject = std::pair<fs::path, fs::path>;
  std::expected<std::vector<fs::path>, std::string>
  compile(std::vector<SourceObject> sources_objects,
          const compiler::Compiler &compiler,
          fs::file_time_type manifest_lastwrite) {
    std::vector<fs::path> objects{};
    objects.reserve(sources_objects.size());

    for (auto &[src, obj] : std::move(sources_objects)) {

      std::error_code ec;
      if (!fs::exists(obj, ec) || object_is_stale(obj, manifest_lastwrite)) {
        log::compile(obj);
        if (const auto parent = obj.parent_path(); !parent.empty()) {
          fs::create_directories(parent, ec);
          if (ec)
            return std::unexpected{"Error in creating build directory"};
        }

        if (auto result = compile_source(src, obj, compiler); !result)
          return std::unexpected{std::move(result).error()};

      } else
        log::cache_hit(obj);

      objects.push_back(std::move(obj));
    }

    return objects;
  }
} // namespace drum::builder_cmd::compile
