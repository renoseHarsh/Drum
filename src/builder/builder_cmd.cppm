export module builder_cmd;

import std;

export import manifest;

export namespace drum::builder_cmd {
  struct BuildArgs {
    bool release{};
  };
  std::expected<void, std::string> execute(const BuildArgs &args,
                                           const manifest::Manifest &manifest);
}; // namespace drum::builder_cmd
