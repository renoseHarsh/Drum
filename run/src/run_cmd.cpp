module;

#include <errno.h>
#include <unistd.h>

module run_cmd;

import std;

import builder_cmd;

namespace drum::run_cmd {
  std::expected<void, std::string> execute(const RunArgs &) {
    if (auto result = builder_cmd::execute({}); !result.has_value()) {
      return result;
    }

    std::vector<char *> argv = {const_cast<char *>("build/main"), nullptr};

    execvp("build/main", argv.data());

    int err = errno;
    return std::unexpected{
        std::error_code{err, std::generic_category()}.message()};
  }
} // namespace drum::run_cmd
