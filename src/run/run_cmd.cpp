module;

#include <errno.h>
#include <unistd.h>

module run_cmd;

import std;

import manifest;

namespace drum::run_cmd {
  std::expected<void, std::string> execute(const RunArgs &args,
                                           const manifest::Manifest &manifest) {
    const std::string output = std::format(
        "build/{}/{}", args.release ? "release" : "debug", manifest.name);
    const std::vector<char *> argv{const_cast<char *>(output.data()), nullptr};

    execvp(output.data(), argv.data());

    const int err = errno;
    return std::unexpected{
        std::error_code{err, std::generic_category()}.message()};
  }
} // namespace drum::run_cmd
