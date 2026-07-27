module;

#include <errno.h>
#include <unistd.h>

module run_cmd;

import std;

import builder_cmd;

namespace drum::run_cmd {
  std::expected<void, std::string> execute(const RunArgs &) {
    if (const auto result = builder_cmd::execute({}); !result) {
      return result;
    }

    const std::vector<char *> argv{const_cast<char *>("build/main"), nullptr};

    execvp("build/main", argv.data());

    const int err = errno;
    return std::unexpected{
        std::error_code{err, std::generic_category()}.message()};
  }
} // namespace drum::run_cmd
