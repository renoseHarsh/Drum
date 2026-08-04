module builder_cmd:archive;

import std;

import :process;
import :log;
import :cache;

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
    if (auto result = cache::ensure_build_dir(); !result)
      return result;

    if (objects.empty()) {
      return {};
    }

    fs::path output_path = "build";
    output_path /= output;
    output_path.replace_extension(".a");

    if (!cache::output_is_stale(output_path, objects)) {
      log::cache_hit(output_path);
      return {};
    }

    std::error_code ec;
    fs::remove(output_path, ec);
    if (ec)
      return std::unexpected{"unexpected error " + ec.message()};

    log::archive(output_path);
    return archive_objects(objects, output_path);
  }

} // namespace drum::builder_cmd::archive
