export module executor;
export import commands;
import std;

export namespace drum::executor {
  std::optional<std::string> execute_new(const commands::NewArgs &cmd);
} // namespace drum::executor
