module builder_cmd:dependency;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::dependency {
  namespace {
    using Dependencies = std::vector<fs::path>;
  }
  std::optional<std::tuple<std::string, Dependencies>>
  get_dependencies(const fs::path &dependency_filename) {
    std::ifstream dependency_file{dependency_filename};
    if (!dependency_file) {
      return std::nullopt;
    }

    std::string dependency_text{std::istreambuf_iterator<char>{dependency_file},
                                std::istreambuf_iterator<char>{}};

    auto left{0uz};
    for (auto right{0uz}; right < dependency_text.size(); ++right) {
      if (dependency_text[right] == '\\' &&
          right + 1 < dependency_text.size() &&
          dependency_text[right + 1] == '\n') {
        dependency_text[left++] = ' ';
        dependency_text[left++] = ' ';
        right++;
        continue;
      }
      const auto c = dependency_text[right];
      dependency_text[left++] =
          std::iscntrl(static_cast<unsigned char>(c)) ? ' ' : c;
    }
    dependency_text.resize(left);

    auto colon = dependency_text.find(":");
    if (colon == std::string::npos)
      return std::nullopt;

    const auto start = dependency_text.find_first_not_of(' ');
    std::string target = dependency_text.substr(start, colon - start);
    target.erase(target.find_last_not_of(' ') + 1);
    std::string_view dependency_view =
        std::string_view{dependency_text}.substr(colon + 1);
    auto filter_view = dependency_view | std::views::split(' ') |
                       std::views::filter([](const auto &r) {
                         return !std::ranges::empty(r);
                       });
    auto dependency_n = std::ranges::distance(filter_view);
    Dependencies list{};
    list.reserve(static_cast<std::size_t>(dependency_n));

    std::ranges::transform(
        filter_view, std::back_inserter(list), [](const auto &r) {
          return fs::path{std::string_view{r.begin(), r.end()}};
        });

    return std::tuple{std::move(target), std::move(list)};
  }

} // namespace drum::builder_cmd::dependency
