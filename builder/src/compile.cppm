module builder_cmd:compile;

import std;

import :process;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile {

  namespace {
    std::expected<void, std::string> compile_source(const fs::path &src,
                                                    const fs::path &obj) {
      const std::vector<std::string> args{"clang++", "-c", src.string(),
                                          "-Isrc/",  "-o", obj.string()};
      return process::run_process(args);
    }
  } // namespace

  std::expected<std::vector<fs::path>, std::string>
  compile(const std::vector<fs::path> &sources) {

    std::vector<fs::path> objects{};

    for (const auto &source : sources) {
      const auto obj =
          ("build" / source.lexically_relative("src/")).replace_extension(".o");

      fs::create_directories(obj.parent_path());

      const auto result = compile_source(source, obj);
      if (!result) {
        return std::unexpected{std::move(result).error()};
      }

      objects.push_back(std::move(obj));
    }

    return objects;
  }
} // namespace drum::builder_cmd::compile
