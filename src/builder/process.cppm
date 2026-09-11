module;

#include <crt_externs.h>
#include <errno.h>
#include <spawn.h>
#include <sys/wait.h>
#include <unistd.h>

module builder_cmd:process;

import std;

namespace drum::builder_cmd::process {
  enum class Stdout { inherit, capture };

  template <std::ranges::input_range R>
  std::expected<std::optional<std::string>, std::string>
  run_process(const std::string &executable, const R &args,
              Stdout mode = Stdout::inherit) {
    std::vector<char *> argv;

    argv.push_back(const_cast<char *>(executable.c_str()));

    std::ranges::transform(
        args, std::back_inserter(argv),
        [](const std::string &arg) { return const_cast<char *>(arg.c_str()); });

    argv.push_back(nullptr);

    char **envp = *_NSGetEnviron();

    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_t *actions_ptr = nullptr;

    int pipe_fds[2];
    if (mode == Stdout::capture) {
      if (pipe(pipe_fds) == -1) [[unlikely]] {
        return std::unexpected{
            std::error_code{errno, std::generic_category()}.message()};
      }

      if (int err = posix_spawn_file_actions_init(&actions)) [[unlikely]] {
        close(pipe_fds[0]);
        close(pipe_fds[1]);

        return std::unexpected{
            std::error_code{err, std::generic_category()}.message()};
      }

      actions_ptr = &actions;

      if (int err = posix_spawn_file_actions_adddup2(
              &actions, pipe_fds[1], STDOUT_FILENO)) [[unlikely]] {
        posix_spawn_file_actions_destroy(&actions);
        close(pipe_fds[0]);
        close(pipe_fds[1]);

        return std::unexpected{
            std::error_code{err, std::generic_category()}.message()};
      }

      if (const int err = posix_spawn_file_actions_addclose(
              &actions, pipe_fds[0])) [[unlikely]] {

        posix_spawn_file_actions_destroy(&actions);
        close(pipe_fds[0]);
        close(pipe_fds[1]);

        return std::unexpected{
            std::error_code{err, std::generic_category()}.message()};
      }

      if (const int err = posix_spawn_file_actions_addclose(
              &actions, pipe_fds[1])) [[unlikely]] {

        posix_spawn_file_actions_destroy(&actions);
        close(pipe_fds[0]);
        close(pipe_fds[1]);

        return std::unexpected{
            std::error_code{err, std::generic_category()}.message()};
      }
    }

    pid_t pid;

    if (const int err = posix_spawnp(&pid, argv[0], actions_ptr, nullptr,
                                     argv.data(), envp)) {
      if (mode == Stdout::capture) {
        posix_spawn_file_actions_destroy(&actions);
        close(pipe_fds[0]);
        close(pipe_fds[1]);
      }
      return std::unexpected{
          std::error_code{err, std::generic_category()}.message()};
    }

    if (mode == Stdout::capture) {
      posix_spawn_file_actions_destroy(&actions);
      close(pipe_fds[1]);
    }

    std::string output{};
    if (mode == Stdout::capture) {
      char buffer[0x1000];

      for (;;) {
        const auto count = read(pipe_fds[0], buffer, sizeof(buffer));

        if (count == -1) [[unlikely]] {
          close(pipe_fds[0]);
          return std::unexpected{
              std::error_code{errno, std::generic_category()}.message()};
        }

        if (count == 0)
          break;

        if (count == EINTR)
          continue;

        output.append(buffer, static_cast<std::size_t>(count));
      }

      close(pipe_fds[0]);
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

      if (mode == Stdout::capture)
        return output;

      return std::nullopt;
    }

    return std::unexpected{"unexpected error"};
  }
} // namespace drum::builder_cmd::process
