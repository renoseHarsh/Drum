module builder_cmd:cache;

import std;

import :depfile;

import compile_unit;

namespace fs = std::filesystem;

namespace drum::builder_cmd::cache {

  namespace {

    using timestamp = fs::file_time_type;

    std::optional<timestamp> modified_at(const fs::path &path) {
      std::error_code ec{};
      const auto time = fs::last_write_time(path, ec);
      if (ec)
        return std::nullopt;

      return time;
    }

    bool missing_or_newer(const fs::path &path, timestamp than) {
      const auto time = modified_at(path);
      return !time || *time > than;
    }

    bool depfile_is_stale(const fs::path &object, timestamp object_time) {
      auto dep_path{object};
      dep_path.replace_extension(".d");

      const auto parsed = depfile::parse(dep_path);
      if (!parsed)
        return true;

      const auto &[target, dependencies] = *parsed;
      if (target != object)
        return true;

      return std::ranges::any_of(dependencies, [&](const fs::path &dependency) {
        return missing_or_newer(dependency, object_time);
      });
    }

  } // namespace

  std::expected<void, std::string> ensure_build_dir() {
    std::error_code ec;
    fs::create_directories("build", ec);
    if (ec)
      return std::unexpected{"unexpected error " + ec.message()};
    return {};
  }

  bool output_is_stale(const fs::path &output,
                       const std::vector<fs::path> &inputs) {
    std::error_code ec;
    if (!fs::exists(output, ec))
      return true;

    const auto output_last_created = fs::last_write_time(output, ec);
    if (ec)
      return true;

    return std::ranges::any_of(inputs, [&](const auto &input) {
      const auto input_last_write = fs::last_write_time(input, ec);
      return ec || input_last_write > output_last_created;
    });
  }

  bool unit_is_stale(const compile_unit::TranslationUnit &unit,
                     timestamp manifest_time) {
    const auto object_time = modified_at(unit.object);
    if (!object_time)
      return true;

    if (unit.provides_bmi() && !modified_at(unit.bmi_path()))
      return true;

    if (missing_or_newer(unit.source, *object_time))
      return true;

    if (manifest_time > *object_time)
      return true;

    if (std::ranges::any_of(unit.dependency_bmis, [&](const auto &bmi) {
          return missing_or_newer(bmi.path, *object_time);
        }))
      return true;

    return depfile_is_stale(unit.object, *object_time);
  }

} // namespace drum::builder_cmd::cache
