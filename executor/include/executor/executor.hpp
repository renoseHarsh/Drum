#ifndef DRUM_EXECUTOR_HPP
#define DRUM_EXECUTOR_HPP

#include "commands/commands.hpp"
#include <optional>
#include <string>

namespace drum::executor {
  std::optional<std::string> execute(const commands::NewArgs &cmd);
}

#endif // !DRUM_EXECUTOR_HPP
