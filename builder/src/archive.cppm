module builder_cmd:archive;

import std;

import :process;
import :log;

namespace fs = std::filesystem;

namespace drum::builder_cmd::archive {
  namespace {
    std::expected<void, std::string>
    archive_objects(const std::vector<fs::path> &objects,
                    const fs::path &output) {
      std::vector<std::string> args{"ar"};
      args.reserve(objects.size() + 3);

      args.push_back("rcs");
      args.push_back(output.string());
      std::ranges::transform(objects, std::back_inserter(args),
                             [](const fs::path &p) { return p.string(); });

      return process::run_process(args);
    }
  } // namespace

  std::expected<void, std::string> archive(const std::vector<fs::path> &objects,
                                           std::string_view output) {
    std::error_code ec;
    fs::create_directories("build", ec);
    if (ec)
      return std::unexpected{"unexpected error " + ec.message()};

    if (objects.empty()) {
      return {};
    }

    fs::path output_path = "build";
    output_path /= output;
    output_path.replace_extension(".a");

    bool needs_archive = !fs::exists(output_path, ec);
    if (!needs_archive) {
      const auto output_last_created = fs::last_write_time(output_path, ec);
      if (ec || std::ranges::any_of(objects, [&](const auto &obj) {
            return fs::last_write_time(obj, ec) > output_last_created;
          }))
        needs_archive = true;
    }

    if (needs_archive) {
      fs::remove(output_path, ec);
      log::archive(output_path);
      return archive_objects(objects, output_path);
    }
    log::cache_hit(output_path);

    return {};
  }

} // namespace drum::builder_cmd::archive
