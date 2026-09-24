module mod_deps:scan_deps;

import std;

import process;
import glaze;

namespace fs = std::filesystem;

namespace drum::builder_cmd::mod_deps::scan_deps {

  std::expected<std::string, std::string> scan_deps(fs::path comp_db_file) {
    return process::run_process("clang-scan-deps",
                                std::array<std::string, 3>{
                                    "-format=p1689", "-compilation-database",
                                    comp_db_file.string()},
                                process::Stdout::capture)
        .and_then([](std::optional<std::string> output)
                      -> std::expected<std::string, std::string> {
          if (!output) [[unlikely]]
            return std::unexpected{"No output produced by clang-scan-deps"};

          return std::move(*output);
        });
  }

  [[nodiscard]]
  std::expected<glaze::p1689_types::DependencyInfo, std::string>
  parse(std::string raw_json) {
    glaze::p1689_types::DependencyInfo info;
    if (auto error = glaze::read_json(info, raw_json)) [[unlikely]] {
      std::string buffer;
      return std::unexpected{glaze::format_error(error, buffer)};
    }

    return info;
  }

} // namespace drum::builder_cmd::mod_deps::scan_deps