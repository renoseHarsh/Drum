#ifndef DRUM_EXECUTOR_HPP
#define DRUM_EXECUTOR_HPP

#include "commands/commands.hpp"

namespace drum::executor {
  void execute(const commands::NewArgs &cmd);
}

#endif // !DRUM_EXECUTOR_HPP
