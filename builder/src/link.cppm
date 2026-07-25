module;

#include <crt_externs.h>
#include <spawn.h>
#include <sys/wait.h>

module builder_cmd:link;

import std;

namespace fs = std::filesystem;

namespace drum::builder_cmd::link {

  namespace {
    std::expected<void, std::string>
    link_objects(const std::vector<fs::path> &objects, const fs::path &output) {
      std::vector<std::string> args = {"clang++"};
      args.reserve(objects.size() + 3);

      std::ranges::transform(objects, std::back_inserter(args),
                             [](const fs::path &p) { return p.string(); });

      args.push_back("-o");
      args.push_back(output.string());

      std::vector<char *> argv(args.size() + 1);
      std::ranges::transform(args, argv.begin(),
                             [](std::string &arg) { return arg.data(); });
      argv.back() = nullptr;

      char **envp = *_NSGetEnviron();

      pid_t pid;
      if (int err = posix_spawnp(&pid, "clang++", nullptr, nullptr, argv.data(),
                                 envp)) {
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

  std::expected<void, std::string> link(const std::vector<fs::path> &objects) {
    fs::create_directories("build");
    fs::path output{"build/main"};

    if (auto result = link_objects(objects, output); !result.has_value()) {
      return std::unexpected{std::move(result).error()};
    }

    return {};
  }
} // namespace drum::builder_cmd::link
