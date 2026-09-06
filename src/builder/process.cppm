module;

#include <crt_externs.h>
#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>

module builder_cmd:process;

import std;

namespace drum::builder_cmd::process {
  std::expected<void, std::string>
  run_process(const std::string &executable,
              std::span<const std::string> invocation = {},
              std::span<const std::string> args = {}) {

    std::vector<char *> argv{};
    argv.reserve(invocation.size() + args.size() + 2);

    auto to_c_str = std::views::transform(
        [](const std::string &str) { return const_cast<char *>(str.c_str()); });

    argv.push_back(const_cast<char *>(executable.c_str()));
    argv.append_range(invocation | to_c_str);
    argv.append_range(args | to_c_str);
    argv.push_back(nullptr);

    char **envp = *_NSGetEnviron();
    pid_t pid;
    if (const int err =
            posix_spawnp(&pid, argv[0], nullptr, nullptr, argv.data(), envp)) {
      return std::unexpected{
          std::error_code{err, std::generic_category()}.message()};
    }

    int wstatus;
    if (const int status = waitpid(pid, &wstatus, 0); status == -1) {
      return std::unexpected{
          std::error_code{errno, std::generic_category()}.message()};
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
