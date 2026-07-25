module builder_cmd:discover;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::discover {
  std::vector<fs::path> discover() {
    std::vector<fs::path> sources =
        fs::recursive_directory_iterator{"src"} |
        std::views::filter([](const fs::directory_entry &entry) {
          return entry.is_regular_file() && entry.path().extension() == ".cpp";
        }) |
        std::views::transform(
            [](const fs::directory_entry &entry) { return entry.path(); }) |
        std::ranges::to<std::vector<fs::path>>();

    return sources;
  }
} // namespace drum::builder_cmd::discover
