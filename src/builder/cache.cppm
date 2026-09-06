module builder_cmd:cache;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::cache {
  std::expected<void, std::string> ensure_build_dir() {
    std::error_code ec;
    fs::create_directories("build", ec);
    if (ec)
      return std::unexpected{"unexpected error " + ec.message()};
    return {};
  }

  bool output_is_stale(const fs::path &output,
                       const std::vector<fs::path> &inputs) {
    std::error_code ec;
    if (!fs::exists(output, ec))
      return true;

    const auto output_last_created = fs::last_write_time(output, ec);
    if (ec)
      return true;

    return std::ranges::any_of(inputs, [&](const auto &input) {
      const auto input_last_write = fs::last_write_time(input);
      return ec || input_last_write > output_last_created;
    });
  }
} // namespace drum::builder_cmd::cache
