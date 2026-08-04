module builder_cmd:link;

import std;

import :process;
import :log;

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

    std::error_code ec;
    fs::create_directories("build", ec);
    if (ec)
      return std::unexpected{"unexpected error " + ec.message()};

    if (objects.empty()) {
      return {};
    }

    const fs::path output_path = fs::path{"build"} / fs::path{output};

    bool needs_link = !fs::exists(output_path, ec);
    if (!needs_link) {
      const auto output_last_created = fs::last_write_time(output_path, ec);
      if (ec || std::ranges::any_of(objects, [&](const auto &obj) {
            return fs::last_write_time(obj, ec) > output_last_created;
          }))
        needs_link = true;
    }

    if (needs_link) {
      log::link(output_path);
      return link_objects(objects, output_path);
    }
    log::cache_hit(output_path);

    return {};
  }
} // namespace drum::builder_cmd::link
