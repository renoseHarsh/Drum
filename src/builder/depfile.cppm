module builder_cmd:depfile;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::depfile {
  namespace {
    using Dependencies = std::vector<fs::path>;

    std::string_view trim(std::string_view text) {
      const auto first = text.find_first_not_of(" \t");
      if (first == std::string_view::npos)
        return {};

      const auto last = text.find_last_not_of(" \t");
      return text.substr(first, last - first + 1);
    }
  } // namespace
    //
  std::optional<std::tuple<fs::path, Dependencies>>
  parse(const fs::path &depfile_path) {
    std::ifstream file{depfile_path};
    if (!file)
      return std::nullopt;

    std::string text{std::istreambuf_iterator<char>{file},
                     std::istreambuf_iterator<char>{}};

    std::ranges::replace(text, '\\', ' ');
    std::ranges::replace(text, '\n', ' ');

    auto colon = text.find(":");
    if (colon == std::string::npos)
      return std::nullopt;

    auto target = trim(std::string_view{text}.substr(0, colon));

    Dependencies dependencies{};

    std::istringstream stream{text.substr(colon + 1)};
    for (std::string path; stream >> path;)
      dependencies.emplace_back(std::move(path));

    return std::tuple{target, std::move(dependencies)};
  }

} // namespace drum::builder_cmd::depfile
