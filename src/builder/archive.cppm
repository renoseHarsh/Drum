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
      std::vector<std::string> args;
      args.reserve(objects.size() + 2);

      args.emplace_back("rcs");
      args.push_back(output.string());

      std::ranges::transform(
          objects, std::back_inserter(args),
          [](const fs::path &object) { return object.string(); });

      return process::run_process("ar", args).transform([](auto &&) {});
    }
  } // namespace

  std::expected<void, std::string> archive(const std::vector<fs::path> &objects,
                                           fs::path output_path) {
    if (auto result = cache::ensure_build_dir(); !result)
      return result;

    if (objects.empty()) {
      return {};
    }

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
