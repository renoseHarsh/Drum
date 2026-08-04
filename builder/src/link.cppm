module builder_cmd:link;

import std;

import :process;
import :log;
import :cache;

namespace fs = std::filesystem;

namespace drum::builder_cmd::link {

  namespace {
    std::expected<void, std::string>
    link_objects(const std::vector<fs::path> &objects, const fs::path &output) {
      std::vector<std::string> args{"clang++"};
      args.reserve(objects.size() + 3);

      std::ranges::transform(objects, std::back_inserter(args),
                             [](const fs::path &p) { return p.string(); });
      args.push_back("-o");
      args.push_back(output.string());

      return process::run_process(args);
    }
  } // namespace

  std::expected<void, std::string> link(const std::vector<fs::path> &objects,
                                        std::string_view output) {
    if (auto result = cache::ensure_build_dir(); !result)
      return result;

    if (objects.empty()) {
      return {};
    }

    fs::path output_path = "build";
    output_path /= output;

    if (!cache::output_is_stale(output_path, objects)) {
      log::cache_hit(output_path);
      return {};
    }

    log::link(output_path);
    return link_objects(objects, output_path);
  }
} // namespace drum::builder_cmd::link
