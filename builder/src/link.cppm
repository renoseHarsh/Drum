module builder_cmd:link;

import std;

import :process;

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

  std::expected<void, std::string> link(const std::vector<fs::path> &objects) {
    fs::create_directories("build");
    const fs::path output{"build/main"};

    if (const auto result = link_objects(objects, output); !result) {
      return std::unexpected{std::move(result).error()};
    }

    return {};
  }
} // namespace drum::builder_cmd::link
