export module run_cmd;

import std;

export namespace drum::run_cmd {
  struct RunArgs {};
  std::expected<void, std::string> execute(const RunArgs &args);
}; // namespace drum::run_cmd
