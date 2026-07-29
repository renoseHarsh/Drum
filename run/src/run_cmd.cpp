module;

#include <errno.h>
#include <unistd.h>

module run_cmd;

import std;

import manifest;

namespace drum::run_cmd {
  std::expected<void, std::string> execute(const RunArgs &,
                                           const manifest::Manifest &) {
    const std::vector<char *> argv{const_cast<char *>("build/main"), nullptr};

    execvp("build/main", argv.data());

    const int err = errno;
    return std::unexpected{
        std::error_code{err, std::generic_category()}.message()};
  }
} // namespace drum::run_cmd
