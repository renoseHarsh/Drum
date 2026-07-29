export module builder_cmd;

import std;

export import manifest;

export namespace drum::builder_cmd {
  struct BuildArgs {};
  std::expected<void, std::string> execute(const BuildArgs &,
                                           const manifest::Manifest &manifest);
}; // namespace drum::builder_cmd
