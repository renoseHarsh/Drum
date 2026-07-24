export module builder_cmd;

import std;

export namespace drum::builder_cmd {
  struct BuildArgs {};
  std::expected<void, std::string> execute(const BuildArgs &args);
}; // namespace drum::builder_cmd
