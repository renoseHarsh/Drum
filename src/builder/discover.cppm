module builder_cmd:discover;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::discover {
  std::expected<std::vector<fs::path>, std::string> discover() {
    std::vector<fs::path> sources;

    std::error_code ec;

    fs::recursive_directory_iterator it{
        "src", fs::directory_options::skip_permission_denied, ec};

    if (ec)
      return std::unexpected{ec.message()};

    const fs::recursive_directory_iterator end{};

    while (it != end) {
      if (it->is_regular_file(ec)) {
        if (it->path().extension() == ".cpp")
          sources.push_back(it->path());
      } else if (ec) {
        return std::unexpected{"unexpected error: " + ec.message()};
      }

      it.increment(ec);
      if (ec)
        return std::unexpected{"unexpected error: " + ec.message()};
    }

    return sources;
  }
} // namespace drum::builder_cmd::discover
