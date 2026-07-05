export module executor;
import commands;
import std;

namespace drum::executor {
  export std::optional<std::string> execute(const commands::NewArgs &cmd);
} // namespace drum::executor
