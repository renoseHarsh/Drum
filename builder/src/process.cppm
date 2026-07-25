module;

#include <crt_externs.h>
#include <spawn.h>
#include <sys/wait.h>

module builder_cmd:process;

import std;

namespace drum::builder_cmd::process {
  std::expected<void, std::string> run_process(std::vector<std::string> &args) {
    std::vector<char *> argv(args.size() + 1);

    std::ranges::transform(args, argv.begin(),
                           [](std::string &arg) { return arg.data(); });
    argv.back() = nullptr;

    char **envp = *_NSGetEnviron();
    pid_t pid;
    if (int err =
            posix_spawnp(&pid, argv[0], nullptr, nullptr, argv.data(), envp)) {
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
} // namespace drum::builder_cmd::process
