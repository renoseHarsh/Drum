export module run_cmd;

import std;

export import manifest;

export namespace drum::run_cmd {
  struct RunArgs {
    bool release{};
  };
  std::expected<void, std::string> execute(const RunArgs &args,
                                           const manifest::Manifest &);
}; // namespace drum::run_cmd
