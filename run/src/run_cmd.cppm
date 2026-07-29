export module run_cmd;

import std;

export import manifest;

export namespace drum::run_cmd {
  struct RunArgs {};
  std::expected<void, std::string> execute(const RunArgs &,
                                           const manifest::Manifest &);
}; // namespace drum::run_cmd
