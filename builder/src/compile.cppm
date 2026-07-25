module;

#include <spawn.h>
#include <sys/wait.h>

module builder_cmd:compile;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::compile {

  namespace {
    std::expected<void, std::string> compile_source(const fs::path &src,
                                                    const fs::path &obj) {

      std::vector<std::string> args = {"clang++", "-c", src.string(), "-o",
                                       obj.string()};
      std::vector<char *> argv(args.size() + 1);
      std::ranges::transform(args, argv.begin(),
                             [](std::string &arg) { return arg.data(); });
      argv.back() = nullptr;

      pid_t pid;
      if (int err = posix_spawnp(&pid, "clang++", nullptr, nullptr, argv.data(),
                                 nullptr)) {
        return std::unexpected{std::strerror(err)};
      }

      int wstatus;
      if (int status = waitpid(pid, &wstatus, 0); status == -1) {
        return std::unexpected{std::strerror(status)};
      }

      if (WIFEXITED(wstatus)) {
        if (WEXITSTATUS(wstatus)) {
          return std::unexpected{std::string{}};
        }

        return {};
      }

      return std::unexpected{"unexpected error"};
    }
  } // namespace

  std::expected<std::vector<fs::path>, std::string>
  compile(const std::vector<fs::path> &sources) {

    std::vector<fs::path> objects{};

    for (const auto &source : sources) {
      auto obj =
          ("build" / source.lexically_relative("src/")).replace_extension(".o");

      auto dir = obj;
      dir.remove_filename();
      fs::create_directories(dir);

      auto result = compile_source(source, obj);
      if (!result.has_value()) {
        return std::unexpected{std::move(result).error()};
      }

      objects.push_back(std::move(obj));
    }

    return objects;
  }
} // namespace drum::builder_cmd::compile
